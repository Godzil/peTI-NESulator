"""
Simple peTI-NESulator game database importer

This python application will generate a XML file on the output with all information needed to import in the peTI-NESulator
game database.
"""

import sys, md5, sha, urllib, urlparse


def get_page(url, post_data=None, headers=()):
    """
    Helper method that gets the given URL, handling headers
    """
    opener = urllib.URLopener()
    for k, v in headers:
        opener.addheader(k, v)
    try:
        f = opener.open(url, post_data)
    except IOError, e:
        if e[1] == 302:
            return '<html></html>'
        else:
            raise
    return f.read()

if __name__ == '__main__':
    #print "<gamelist>"
    for filename in sys.argv[1:]:
        f = open(filename)
    
        
        try:
        
            fs = f.read()
            if fs[0:4] == "NES%c" % 0x1A:
                Flags = ord(fs[6]) & 0x0F;
                DiskDude = 0
                if fs[7:16] == "DiskDude!":
                    DiskDude = 1
                    
                mapperID = ord(fs[6]) >> 4
                if DiskDude == 0:
                    mapperID = mapperID | (ord(fs[7]) & 0xF0)
                    
                prgsize = ord(fs[4]) * 16 * 1024
                chrsize = ord(fs[5]) *  8 * 1024
                mirror = 0
                if Flags & 0x01:
                    mirror = 1
                    
                sram = 0
                if Flags & 0x02:
                    sram = 1
                    
                Trainer = 0               
                if Flags & 0x04:
                    Trainer = 1
                
                print " <game>"
                print "  <name>%s</name>" % filename
                print "  <sha>%s</sha>" % sha.new(fs).hexdigest()
                print "  <md5>%s</md5>" % md5.new(fs).hexdigest()
                print "  <mapperID>%d</mapperID>" % mapperID
                print "  <prgsize>%d</prgsize>" % prgsize
                print "  <chrsize>%d</chrsize>" % chrsize
                print "  <miror>%d</miror>" % mirror
                print "  <sram>%d</sram>" % sram
                print "  <trainer>%d</trainer>" % Trainer
                print "  <diskdude>%d</diskdude>" % DiskDude
                print " </game>"
                 
                 
                #will fill the DB :
                url = "http://127.0.0.1/~mtrapier/nesstat/add.php"
                
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
                
                print html
        finally:
            f.close()
            
    #print "</gamelist>"
