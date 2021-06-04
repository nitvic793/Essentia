/****
 * Generated file. DO NOT MODIFY  
 */
 
% for s in header_files:
#include "${s}"
% endfor

void RegisterComponents()
{
% for s in structs:
    GComponentReflector.RegisterComponent<${s['typename']}>();
% endfor
}

% for s in structs:
void Visit(${s['typename']}* component, IVisitor* visitor)
{
    % for f in s['fields']:
    visitor->Visit("${s['typename']}", MField(component, ${f['fieldName']}));
    % endfor
}

% endfor
