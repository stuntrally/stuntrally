import os

def get_files(dir):
	return [name for name in os.listdir(dir) if os.path.isfile(os.path.join(dir, name))]

tdir = 'translations/stuntrally.srpot';
fpo = get_files(tdir)
#`which python2` ./xml_po_parser.py ./translations-export/pofiles/stuntrally/${loc}.po ../data/gui/core_language_${loc}_tag.xml

for po in fpo:
	p2 = po[0:2]
	if p2 != 'en':
		print('----  '+p2+'  '+po)
		cmd = 'xml_po_parser.py '+tdir+'/'+po+' ../data/gui/core_language_'+p2+'_tag.xml'
		#print(cmd)
		os.system(cmd)
