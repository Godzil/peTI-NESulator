#!/usr/bin/python
"""
Simple peTI-NESulator game database importer

This python application will generate a XML file on the output with all information needed to import in the peTI-NESulator
game database.
"""

import urllib3 as urllib
import io
import sys
import hashlib.md5 as md5
import hashlib.sha as sha


def get_page(theurl, post_data=None):
    """
    Helper method that gets the given URL
    """
    http = urllib.PoolManager()

    req = http.request('POST', theurl, fields=post_data)

    if req.status == 302 or req.status == 200:
        return req.data

    return "Failure"


if __name__ == '__main__':
    for filename in sys.argv[1:]:
        f = io.open(filename)

        try:
            fs = f.read()
            if fs[0:4] == "NES%c" % 0x1A:
                Flags = ord(fs[6]) & 0x0F
                DiskDude = 0
                if fs[7:16] == "DiskDude!":
                    DiskDude = 1

                mapperID = ord(fs[6]) >> 4
                if DiskDude == 0:
                    mapperID = mapperID | (ord(fs[7]) & 0xF0)

                prgsize = ord(fs[4]) * 16 * 1024
                chrsize = ord(fs[5]) * 8 * 1024
                mirror = 0
                if Flags & 0x01:
                    mirror = 1

                sram = 0
                if Flags & 0x02:
                    sram = 1

                Trainer = 0
                if Flags & 0x04:
                    Trainer = 1

                print(" <game>")
                print("  <name>{filename}</name>".format(filename=filename))
                print("  <sha>{sha}</sha>".format(sha=sha.new(fs).hexdigest()))
                print("  <md5>{md5}</md5>".format(md5=md5.new(fs).hexdigest()))
                print("  <mapperID>{id}</mapperID>".format(id=mapperID))
                print("  <prgsize>{size}</prgsize>".format(size=prgsize))
                print("  <chrsize>{size}</chrsize>".format(size=chrsize))
                print("  <miror>{mirror}</miror>".format(mirror=mirror))
                print("  <sram>{sram}</sram>".format(sram=sram))
                print("  <trainer>{trainer}</trainer>".format(trainer=Trainer))
                print("  <diskdude>{diskdude}</diskdude>".format(diskdude=DiskDude))
                print(" </game>")

                # will fill the DB :
                url = "http://127.0.0.1/nesstat/add.php"

                html = get_page(url, urllib.urlencode({
                    'n': filename,
                    'md5': md5.new(fs).hexdigest(),
                    'sha1': sha.new(fs).hexdigest(),
                    'm': mapperID,
                    'prg': prgsize,
                    'chr': chrsize,
                    'mir': mirror,
                    'sram': sram,
                    't': Trainer,
                    'd': DiskDude,
                }))

                print(html)
        finally:
            f.close()

    # print("</gamelist>")
