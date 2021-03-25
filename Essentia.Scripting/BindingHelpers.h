#pragma once

#ifndef WREN_BINDING_HELPERS

#define MWrenGetter(type, name, field, wrenType) void _VM_ ## name ## Get ## field(WrenVM* vm)\
{ \
	const type* value = (const type*)wrenGetSlotForeign(vm, 0);	\
	wrenSetSlot ## wrenType(vm, 0, value->field); \
} 

#define MWrenSetter(type, name, field, wrenType) void _VM_ ## name ## Set ## field(WrenVM* vm)\
{ \
	type* value = (type*)wrenGetSlotForeign(vm, 0);	\
	auto newFieldVal = wrenGetSlot ## wrenType(vm, 1); \
	value->field = (decltype(newFieldVal))newFieldVal; \
} 

#define MWrenGetterDouble(type, name, field) MWrenGetter(type, name, field, Double)
#define MWrenSetterDouble(type, name, field) MWrenSetter(type, name, field, Double)

#define MWrenGetSetterName(type, name, field) _VM_ ## name ## Set ## field
#define MWrenGetGetterName(type, name, field) _VM_ ## name ## Get ## field

#define MWrenRegisterSetter(module, method, class, field) es::Binding::GetInstance().BindMethod(#module, #class, #field "=(_)" , false, method);
#define MWrenRegisterGetter(module, method, class, field) es::Binding::GetInstance().BindMethod(#module, #class, #field, false, method);

#endif // !WREN_BINDING_HELPERS