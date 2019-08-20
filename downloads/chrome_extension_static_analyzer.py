#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import jsbeautifier
import sys
reload(sys)
sys.setdefaultencoding('utf-8')

import re
import json

name = {}
vuls = {}
manifest_vul = {}

jslib_files = {'jquery'}

def processjs(filename):

	for f in jslib_files:
		if f in filename:
			return

	with open(filename, 'r') as f:
		data = f.read()
	res = jsbeautifier.beautify(data)
	with open(filename, 'w') as fp:
		fp.write(res)

	for i in name.keys():
		if i in filename:
			eid = i
			break
	vul = []
	pattern = "\\b(addEventListener)\\s{0,10}\\(['\"]\\b(message)['\"]"
	if re.search(pattern, res):
		vul.append(str(( 'window.addEventListener(message)', filename )))
	if 'runtime.onMessage' in res:
		vul.append(str(( 'runtime.onMessage', filename)))
	if 'runtime.onMessageExternal' in res:
		vul.append(str(( 'runtime.onMessageExternal', filename)))
	if 'runtime.onConnectExternal' in res:
		vul.append(str(( 'runtime.onConnectExternal', filename)))
	if 'location' in res:
		vul.append(str(( 'location', filename)))
	if '.innerHTML' in res:
		vul.append(str(( 'innerHTML', filename)))
	if '.html(' in res:
		vul.append(str(( 'html()', filename)))
	if 'document.write(' in res:
		vul.append(str(( 'document.write()', filename)))
	if 'eval(' in res:
		vul.append(str(( 'eval()', filename)))
	if 'chrome.tabs.executeScript' in res:
		vul.append(str(( 'chrome.tabs.executeScript', filename)))

	if vul:
		if not vuls.get(eid, None): vuls[eid] = []
		vuls[eid].append(vul)


def processmanifest(filename):
	flag = True
	data = json.load(open(filename, 'r'))
	sensitive = ['web_accessible_resources', 'content_security_policy', 'externally_connectable', 'content_scripts']
	eid = filename.split('/')[-3]
	name[eid] = data['name']
	if data.get('update_url', None):
		if data.get('update_url').startswith('http://'):
			if flag:
				manifest_vul[eid] = []
				flag = False
			manifest_vul[eid].append(str(( 'update_url', data.get('update_url') ))) 

	if data.get('permissions', None):
		for i in data.get('permissions'):
			if "//*" in i:
				if flag:
					manifest_vul[eid] = []
					flag = False
				manifest_vul[eid].append(str(( 'permissions', data.get('permissions') ))) 
				break

	for i in sensitive:
		if data.get(i, None):
			if flag:
				manifest_vul[eid] = []
				flag = False
			manifest_vul[eid].append(str(( i, data[i] ))) 

def gci(filepath):
	#遍历filepath下所有文件，包括子目录
	if os.path.exists(os.path.join(filepath,'manifest.json')):
		processmanifest(os.path.join(filepath,'manifest.json'))
	files = os.listdir(filepath)
	for fi in files:
		fi_d = os.path.join(filepath,fi)			
		if os.path.isdir(fi_d):
			gci(fi_d)				  
		else:
			if fi_d.endswith('.js') or fi_d.endswith('.html') or fi_d.endswith('.htm'):
				processjs(fi_d)
				

#递归遍历/root目录下所有文件
#gci('./src/')
gci('./Extensions/')

for eid, name in name.iteritems():
	if manifest_vul.get(eid, None):
		print '#'*5+' '+name+' '+'#'*5
		print '\n'.join(manifest_vul[eid])
		print '#'*5+' '+name+' '+'#'*5
	if vuls.get(eid, None):
		print '-'*5+' '+name+' '+'-'*5
		for i in vuls[eid]:
			print '\n'.join(i)
		print '-'*5+' '+name+' '+'-'*5