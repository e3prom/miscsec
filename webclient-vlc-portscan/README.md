## Summary
I started a little experiment after finding out that VLC for Windows when
referencing resources with the 'file://' URI schema in playlist, is leveraging
the operating-system's WebClient service. Under some particular conditions, the
latter try to query files and directories properties via the WebDAV extension.

When a SMB share cannot be accessed (or doesn't exists), the WebDAV extension
is trigged, targeting the referenced host. The WebDAV server could redirect the
client using HTTP replies with the '301 Moved' method. The victim's client seemto
 blindly redirect to the location as pointed by the 'Location:' header field's
 value. The redirects could be leveraged to perform CSRF attacks and even basic
  TCP port scan (see Proof of Concept below).

## Proof of Concept (Experiment)
I wrote a quick and dirty [PoC](nasty-webdav.py) simulating a WebDAV server.
The server can handle multiples connections, is listening by default on tcp/80
and can redirect the victim to a range of TCP ports of your choosing. This PoC
could be used to abuse the WebDAV client in various way, and can also lead the
victim in performing a basic TCP scan. By observing returns from the client,
you can infer which ports is filtered outbound.

You can find a few additional files to demonstrate the issue with VLC. At this
time of witting, it does affect all version of the software, including the
latest development version. By opening the "malicious" playlist, the victim's
will query the WebDAV server (don't forget to point the resource to your IP
address), and it will redirect the client to target you specify in the
'self.target' variable (see below for full instructions).

Step to reproduce (VLC):
 1. Edit [nasty-webdav-server.py](nasty-webdav-server.py):
    * ```self.target``` To point to your target IP address, where to observe
       connections attempts from the victim(s) (could point to your WebDAV
       server or another host).
    * ```self.port_start``` The start port to enumerate from (avoid using port
       80 twice).
    * ```self.port_end``` The last port to enumerate to.
 2. Install and setup the SMB server on your attacker's host:
     ```
     # apt-get install samba
     # vi /etc/samba/smb.conf
       [lab]
       comment = R/W share
       browseable = yes
       path = /var/samba
       guest ok = yes
       read only = no
       create mask = 0777
     # /etc/init.d/smbd restart
     ```
  3. Place [playlist-loop.xspf](playlist-loop.xspf) on your SMB share
     (here ```/var/samba```) and edit:
     * The first track's location to point to your attacker's SMB server
       but on a **invalid share** (necessary to trigger the WebDAV extension)
     * The second track's location to point to your attacker's SMB server
       and this playlist file. The victim's will loop through it, giving us
       enough time for the redirects (and port scan) to occurs.
  4. Edit the [playlist.xspf](playlist.xspf):
     * Modify the playlist's track location to point to your valid SMB share,
       holding the ```playlist-loop.xspf``` playlist above.
  5. Run the PoC WebDAV server:
     ```
     $ chmod 755 nasty-webdav-server.py
     $ sudo ./nasty-webdav-server.py
     ```
  6. Open ```playlist.xspf``` file in VLC for Windows.
  7. Use a sniffer or another tool of your choice to observe the connections
     attempts from your victim(s). Pretty neat isn't it?
