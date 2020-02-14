#include "ExtraSignals.h"
#include <TGUI/Signal.hpp>
#include <TGUI/Widget.hpp>
#include <TGUI/Widgets/ChildWindow.hpp>
#include <TGUI/SignalImpl.hpp>

#include <set>

namespace {
	unsigned int lastUniqueSignalId = 0;

	unsigned int generateUniqueId() {
		return ++lastUniqueSignalId;
	}

	template <typename T>
	bool checkParamType(std::initializer_list<std::type_index>::const_iterator type) {
#ifdef TGUI_UNSAFE_TYPE_INFO_COMPARISON
		return strcmp(type->name(), typeid(T).name()) == 0;
#else
		return *type == typeid(T);
#endif
	}
}

namespace tgui {
#define TGUI_SIGNAL_VALUE_CONNECT_DEFINITION(TypeName, Type) \
    unsigned int Signal##TypeName::connect(const Delegate##TypeName& handler) \
    { \
        const auto id = generateUniqueId(); \
        m_handlers[id] = [handler](){ handler(internal_signal::dereference<Type>(internal_signal::parameters[1])); }; \
        return id; \
    } \
    \
    unsigned int Signal##TypeName::connect(const Delegate##TypeName##Ex& handler) \
    { \
        const auto id = generateUniqueId(); \
        m_handlers[id] = [handler, name=m_name](){ handler(getWidget(), name, internal_signal::dereference<Type>(internal_signal::parameters[1])); }; \
        return id; \
    }
	TGUI_SIGNAL_VALUE_CONNECT_DEFINITION(EditorAttack, std::shared_ptr<EditorAttack>)

	unsigned int SignalEditorAttack::validateTypes(std::initializer_list<std::type_index> unboundParameters) const {
		if ((unboundParameters.size() == 1) && checkParamType<std::shared_ptr<EditorAttack>>(unboundParameters.begin()))
			return 1;
		else
			return Signal::validateTypes(unboundParameters);
	}
}