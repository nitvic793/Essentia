/****
 * Generated file. DO NOT MODIFY  
 */

% for s in structs:
foreign class ${s['typename']} {
  construct get(entity) {}
  foreign set(entity)
  % for f in s['fields']:
  foreign ${f['fieldName']}
  foreign ${f['fieldName']}=(rhs)
  % endfor
}

% endfor

