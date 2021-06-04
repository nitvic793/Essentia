/****
 * Generated file. DO NOT MODIFY  
 */

% for s in structs:
foreign class ${s['typename']} {
  construct get(entity) {}
  % for f in s['fields']:
  foreign ${f['fieldName']}
  foreign ${f['fieldName']}=(rhs)
  % endfor
  foreign static [index]
  foreign static count()
  foreign static getEntity(index) 
  foreign static getEntities()
  foreign static getComponents()
}

% endfor

