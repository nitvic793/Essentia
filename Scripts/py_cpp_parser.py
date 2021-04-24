#!/usr/bin/env python
""" 
Usage: call with <filename> <typename>
"""
import sys
import clang.cindex
import clang.cindex as cl
from clang.cindex import CursorKind
from clang.cindex import TokenKind
import ccsyspath
from ctypes.util import find_library
import re
import os 
import fnmatch
from mako.template import Template

clang.cindex.Config.set_library_path(
    'C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\Llvm\\bin')  

match_str="//@Skip\(([A-Za-z\s,_]*)\)"
match_serial_str="//@Serialize\(([A-Za-z\s,_]*)\)"

base_component_classes = ['struct IComponent', 'struct IDrawable', 'struct ILight']
base_system_classes = ['class ISystem']

def get_current_scope(cursor):
    """
    Get the current scope of the current cursor. 
    For example: 
    namespace A {
    namespace B { 
    class C {
        <CURSOR IS IN HERE>
    }; 
    }
    } 
    will return: ["A", "B", "C"] and can be joined to be "A::B::C" 
    Parameters ::
      - cursor: A clang.cindex.Cursor to loop for declaration parents of. 
    Returns ::
      - A list of names of the scopes.
    """
    # Get the parent of the current cursor
    parent = cursor.lexical_parent
    # If the parent is a declartaion type then add it to the end of our scope
    # list otherwise return an empty list
    if (parent.kind.is_declaration()):
        return get_current_scope(parent) + [parent.spelling]
    else:
        return [] 


def find_parent(cursor):
    for c in cursor.get_children():
        if c.kind == CursorKind.CXX_BASE_SPECIFIER:
            return c
    return None

             
def find_game_components(tu):
    """
    Iterate through all tokens in the current TranslationalUnit and look for components
    Parameters ::
        - tu: The TranslationalUnit to search over
        - match_str: The comment string to match to skip component auto registration
    Returns ::
        - A List components and header files
    """
    match_types = [
        cl.CursorKind.STRUCT_DECL, 
        cl.CursorKind.CLASS_DECL, 
        # cl.CursorKind.CXX_BASE_SPECIFIER
        ] 
    tokens = tu.cursor.get_tokens() 
    serializables = []
    struct_map = {}
    skip_struct_map = {}
    skip = False
    serialize = False
    header_files = {}
    for token in tokens:
        match = re.match(match_str, token.spelling)
        if match:
            skip = True
        match = re.match(match_serial_str, token.spelling)
        if match:
            serialize = True  

        cursor = cl.Cursor().from_location(tu, token.location)
        if cursor.spelling in struct_map or cursor.spelling in skip_struct_map:
            continue
        parent = find_parent(cursor)
        if parent is not None and parent.spelling in base_component_classes and serialize:
            serialize = False # Reset flag
            if cursor.kind in match_types:
                if skip:
                    skip_struct_map[cursor.spelling] = True
                    skip = False
                    continue
                # Extract the name, and the scope of the cursor and join them
                # to for the full C++ name.
                name = "::".join(get_current_scope(cursor) + [cursor.spelling])
                # Extract all of the fields (including access_specifiers)
                fields = [{'fieldName': field.spelling, 'fieldType': field.type.spelling,
                           'accessSpecifier': field.access_specifier.name
                           } 
                          for field in cursor.type.get_fields()
                          ]
                
                serializables.append(
                    {'typename': name, 'fields': fields, 'args': None})
                header_files[tu.spelling] = tu.spelling
                parent = None
                struct_map[cursor.spelling] = True
    return serializables, header_files  


def get_clang_TranslationUnit(path="t.cpp", in_args=[], in_str="", options=0):
    """
    Get the TranslationalUnit for the input fule listed: 
    Parameters ::
        - path: The path of the file to parse (not read if in_str is set)
        - in_args: additional arguments to pass to clang
        - in_str: input string to parse instead of the file path.
        - options: clang.cindex.Options 
    Returns ::
        - A TranslationalUnits
    """ 
    # Make sure we are parsing as std c++11
    args = '-x c++ --std=c++17'.split()
    # Add the include files for the standard library.
    syspath = ccsyspath.system_include_paths('clang++')
    incargs = [b'-I' + inc for inc in syspath]
    # turn args into a list of args (in_args may contain more includes)
    args = args + incargs + in_args 
    # Create a clang index to parse into
    index = cl.Index.create() 
    unsaved_files = None
    # If we are parsing from a string instead
    if in_str:
        unsaved_files = [(path, in_str)]
    return index.parse(path, args=args, options=options,
                       unsaved_files=unsaved_files)  




def main():
    includes = ['*.h'] # for files only
    includes = r'|'.join([fnmatch.translate(x) for x in includes])
    path = sys.argv[1]
    files = []
    for (dirpath, dirnames, filenames) in os.walk(path):
        filenames = [os.path.join(dirpath, f) for f in filenames]
        filenames = [f for f in filenames if re.match(includes, f)]
        files.extend(filenames)

    structs = []
    header_files = {}
    for file in files:
        tu = get_clang_TranslationUnit(file)
        result, headers = find_game_components(tu) 
        structs.extend(result)
        header_files.update(headers)

    for struct in structs:
        for field in struct['fields']:
            field['friendlyTypeName'] = field['fieldType']
            field['friendlyTypeName'] = field['fieldType'].replace('::', '')
            field['friendlyTypeName'] = field['friendlyTypeName'].replace(' ', '_')

            if field['fieldType'] == 'float' or 'int' in field['fieldType'] :
                field['wrenType'] = 'Double'
            elif field['fieldType'] == 'bool':
                field['wrenType'] = 'Bool'
            else:
                field['wrenType'] = 'Foreign'
    abspath = os.path.abspath(__file__)
    dname = os.path.dirname(abspath) + '\\'

    compTpl = Template(filename=dname+'visitor.mako')
    wrenTpl = Template(filename=dname+'wren.mako')
    wrenBindingTpl = Template(filename=dname+'wrenBinding.mako')
    headersTpl = Template(filename=dname+'headers.mako')

    print(compTpl.render(structs=structs, header_files=header_files,
                    module_name='CodegenExample'))
    print(wrenTpl.render(structs=structs, header_files=header_files,
                    module_name='CodegenExample'))
    print(wrenBindingTpl.render(structs=structs, header_files=header_files,
                module_name='CodegenExample'))
    print(headersTpl.render(structs=structs, header_files=header_files,
                    module_name='CodegenExample'))

main()