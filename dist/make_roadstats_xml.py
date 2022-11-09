#!/usr/bin/python3
#!/usr/bin/env python3
from xml.dom.minidom import parseString
import os
import re

def get_dirs(dir):
	return [name for name in os.listdir(dir) if os.path.isdir(os.path.join(dir, name))]

pre = '..';  # path
tdir = pre+'/data/tracks'
trks = get_dirs(tdir)
#print(trks)

stats = open('roadstats.xml','w')  # out file
stats.write('<roadstats>\n')

map = {'': 0}  # result map
r = re.compile('[ ,:=\n]+')

times = open(pre+'/config/tracks.ini','r')  # path
for line in times:
	if len(line) > 0 and line[0] >= '0' and line[0] <= '9':
		tr = r.split(line)
		trk = tr[1]
		#print(tr)
		tim = tr[20]
		map[trk] = tim
		print(trk + " " + tim)
times.close()
	
i = 0;
for t in trks:
	if t.find('-') != -1:
		# get road stats
		#print(t)
		file = open(tdir+'/'+t+'/road.xml','r')
		data = file.read()
		file.close()
		
		dom = parseString(data)
		xTag = dom.getElementsByTagName('stats')[0].toxml()
		#print(xTag)
		xNew = xTag.replace('<stats','<s n="'+t+'" t="'+map.get(t,'0')+'"').replace('yaw="0"','').replace('pitch="0" ','').replace('roll="0" ','')
		xNew = xNew.replace('height=','h=').replace('length=','l=').replace('width=','w=').replace('bnkAvg=','ba=').replace('bnkMax=','bm=')
		xNew = xNew.replace('onPipe=','op=').replace('onTer=','ot=').replace('pipes=','p=')
		#print(xNew)
		i += 1
		stats.write(xNew+'\n')

stats.write('</roadstats>')
stats.close()
print('All: '+str(i))
