/****
 * Generated file. DO NOT MODIFY  
 */
#include "pch.h"

% for s in header_files:
#include "${s}"
% endfor

void RegisterBindings()
{
    auto& binding = es::Binding::GetInstance();
% for s in structs:
    binding.BindForeignClass("components", "${s['typename']}", { AllocateComponent<${s['typename']}> , FinalizeType<${s['typename']}> });
    binding.BindMethod("components", "${s['typename']}", "set(_)", false, SetComponent<${s['typename']}>);
    % for f in s['fields']:
    MWrenBindGetterSetter(components, ${s['typename']}, ${f['fieldType']}, ${f['friendlyTypeName']}, ${f['fieldName']}, ${f['wrenType']});
    % endfor
% endfor
}
