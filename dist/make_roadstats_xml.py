from xml.dom.minidom import parseString
import os

def get_dirs(dir):
	return [name for name in os.listdir(dir) if os.path.isdir(os.path.join(dir, name))]

tdir = '../data/tracks';  # path
trks = get_dirs(tdir)
#print trks

stats = open('roadstats.xml','w')  # out file
stats.write('<roadstats>\n');

# champs
champs = open('../config/championships.xml','r')  # path
chdata = champs.read()
champs.close()

chdom = parseString(chdata)
chTags = chdom.getElementsByTagName('times')[0]
chTrks = chTags.getElementsByTagName('track')
#print chTrks

# track time map
map = {'': 0}
for t in chTrks:
	map[t.getAttributeNode('name').nodeValue] = t.getAttributeNode('time').nodeValue
	#print t.toxml()
#print map['J1-T']  # check track time
	
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
