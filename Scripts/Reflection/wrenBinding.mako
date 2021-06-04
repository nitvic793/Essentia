/****
 * Generated file. DO NOT MODIFY  
 */
#include "pch.h"

% for s in header_files:
#include "${s}"
% endfor

void RegisterBindings()
{
    auto& binding = es::ScriptBinding::GetInstance();
% for s in structs:
    binding.BindForeignClass("components", "${s['typename']}", { AllocateComponent<${s['typename']}> , FinalizeType<${s['typename']}> });
    BindComponentHelpers<${s['typename']}>(binding);
    % for f in s['fields']:

    binding.BindMethod("components", "${s['typename']}", "${f['fieldName']}=(_)", false, [](WrenVM* vm)
    {
        ${s['typename']}** comp = (${s['typename']}**)wrenGetSlotForeign(vm, 0);
        % if 'Double' in f['wrenType'] :
        float rhs = (float)wrenGetSlotDouble(vm, 1);
        (*comp)->${f['fieldName']} = rhs;
        % endif
        % if 'Bool' in f['wrenType'] :
        bool rhs = (float)wrenGetSlotBool(vm, 1);
        (*comp)->${f['fieldName']} = rhs;
        % endif
    });

    binding.BindMethod("components", "${s['typename']}", "${f['fieldName']}", false, [](WrenVM* vm)
    {
        ${s['typename']}** comp = (${s['typename']}**)wrenGetSlotForeign(vm, 0);
        % if 'Double' in f['wrenType'] :
        wrenSetSlotDouble(vm, 0, (double)(*comp)->${f['fieldName']});
        % endif
        % if 'Bool' in f['wrenType']:
        wrenSetSlotBool(vm, 0, (*comp)->${f['fieldName']});
        % endif
    });
    % endfor
% endfor
}
