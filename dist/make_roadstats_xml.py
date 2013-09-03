from xml.dom.minidom import parseString
import os
import re

def get_dirs(dir):
	return [name for name in os.listdir(dir) if os.path.isdir(os.path.join(dir, name))]

tdir = '../data/tracks';  # path
trks = get_dirs(tdir)
#print trks

stats = open('roadstats.xml','w')  # out file
stats.write('<roadstats>\n');

map = {'': 0}  # result map
r = re.compile('[ ,:=\n]+')

times = open('../config/tracks.ini','r')  # path
for line in times:
	if len(line) > 0 and line[0] >= '0' and line[0] <= '9':
		tr = r.split(line)
		trk = tr[1]
		tim = tr[len(tr)-2]
		map[trk] = tim
		#print trk + " " + tim
times.close()
	
for t in trks:
	if t != '.git':
		# get road stats
		file = open(tdir+'/'+t+'/road.xml','r')
		data = file.read()
		file.close()
		
		dom = parseString(data)
		xTag = dom.getElementsByTagName('stats')[0].toxml()
		#print xTag
		xNew = xTag.replace('<stats','<stats track="'+t+'" time="'+map.get(t,'0')+'"').replace('yaw="0"','').replace('pitch="0" ','').replace('roll="0" ','')
		#print xNew
		print t
		stats.write(xNew+'\n');

stats.write('</roadstats>');
stats.close();
