#!/usr/bin/python

import sys
from optparse import OptionParser
from xml.etree.ElementTree import ElementTree
import os.path

from wheezy.template.engine import Engine
from wheezy.template.ext.core import CoreExtension
from wheezy.template.loader import FileLoader

script_dir = os.path.dirname(__file__) + os.sep

def parse_file(filename):
    tree = ElementTree()
    tree.parse(filename)
    
    #config_element = tree.find("config")
    classname = tree.getroot().attrib["name"]
    include = tree.findtext("include")
    
    enums = {}
    for enum in tree.findall("enums/enum"):
        elements = list(map(lambda f: f.attrib["name"], enum.getiterator("element")))
        enums[enum.attrib["name"]] = elements

    values = []
    for v_elem in tree.findall('values/value'):
        v = {}
        v['name'] = v_elem.attrib['name']
        v['type'] = v_elem.attrib['type']
        v['default'] = v_elem.attrib['default']
        v['comment'] = v_elem.text
        values.append(v)

    global_vars = []

    for g_elem in tree.findall('global'):
        global_vars.append(g_elem.attrib['name'])
    
    return {"classname":classname,
            "include":include,
            "enums":enums,
            "values":values,
            "filename":filename,
            "globals":global_vars}
                

parser = OptionParser("usage: %prog [options] input_file")
parser.add_option("-H", "--header_ext", dest="header_ext",
                  help="file extension of generated header file", metavar="EXT",
                  default="hh")
parser.add_option("-C", "--source_ext", dest="source_ext",
                  help="file extension of generated source file", metavar="EXT",
                  default="cc")
parser.add_option("-D", "--dir", dest="output_dir",
                  help="output directory", metavar="DIR",
                  default=".")

(options, args) = parser.parse_args()

if len(args) != 1:
    parser.error("incorrect number of arguments")

filename = args[0]

parsed_file = parse_file(filename)
classname = parsed_file["classname"]

template_namespace = {'class_name' : classname,
                      'header_ext' : options.header_ext,
                      'include' : parsed_file['include'],
                      'enums' : parsed_file['enums'],
                      'fields' : parsed_file['values'],
                      'global_vars' : parsed_file['globals'],
                      'config_hash' : hash(open(filename).read())
                      }

engine = Engine(loader=FileLoader([script_dir]), extensions=[CoreExtension()])

generated_warning = '/* WARNING: This file was automatically generated */\n/* Do not edit. */\n'

for filename,extension in [('config_header_template.hh', options.header_ext),
                           ('config_source_template.cc', options.source_ext)
                           ]:
    template_path = '%s/%s' % (script_dir,filename)
    
    infile = os.path.basename(template_path)
    outfile = '%s/%s.%s' % (options.output_dir, classname, extension)

    template = engine.get_template(infile)

    with open(outfile, 'w') as out:
        out.write(generated_warning)
        out.write(template.render(template_namespace))
        
