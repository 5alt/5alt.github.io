import json

def process(filename):
	print (open(filename, 'r').read().strip())

	#data = json.load(open(filename, 'r'))

	for name, value in data.iteritems():
		if not value.get('matches', None) and not value.get('whitelist', None) and (not value.get('dependencies', None) or (value.get('dependencies', None) and len([i for i in value['dependencies'] if 'tabs' in i]))):
			print name

files = ['_api_features.json', '_api_features1.json']

for filename in files:
	process(filename)