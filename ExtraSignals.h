#pragma once

#include <TGUI/Global.hpp>
#include <TGUI/Vector2f.hpp>
#include <TGUI/Animation.hpp>
#include <TGUI/Signal.hpp>
#include <SFML/System/String.hpp>
#include <type_traits>
#include <functional>
#include <typeindex>
#include <memory>
#include <vector>
#include <deque>
#include <map>
#include <memory>
#include "Attack.h"
#include "AudioPlayer.h"
#include "EditorMovablePoint.h"

namespace tgui {
	#define EXTRA_SIGNAL_VALUE_DECLARATION(TypeName, Type) \
    /**************************************************************************************************************************** \
     * @brief Signal to which the user can subscribe to get callbacks from
	 ****************************************************************************************************************************/ \
	class Signal##TypeName : public Signal \
    { \
    public: \
        using Delegate##TypeName = std::function<void(Type)>; \
        using Delegate##TypeName##Ex = std::function<void(std::shared_ptr<Widget>, const std::string&, Type)>; \
        using Signal::connect; \
        \
        /************************************************************************************************************************ \
         * @brief Constructor
		 ************************************************************************************************************************/ \
	Signal##TypeName(std::string&& name) : \
            Signal{std::move(name), 1} \
        { \
        } \
        \
        /************************************************************************************************************************ \
         * @brief Connects a signal handler that will be called when this signal is emitted
		 *
		 * @param handler  Callback function that is given a Vector2f as argument
		 *
		 * @return Unique id of the connection
		 ************************************************************************************************************************/ \
	unsigned int connect(const Delegate##TypeName& handler); \
        \
        /************************************************************************************************************************ \
         * @brief Connects a signal handler that will be called when this signal is emitted
		 *
		 * @param handler  Callback function that is given a pointer to the widget, the name of the signal and a Vector2f as arguments
		 *
		 * @return Unique id of the connection
		 ************************************************************************************************************************/ \
	unsigned int connect(const Delegate##TypeName##Ex& handler); \
        \
        /************************************************************************************************************************ \
         * @internal
		 * @brief Call all connected signal handlers
		 ************************************************************************************************************************/ \
	bool emit(const Widget* widget, Type param) { \
		if (m_handlers.empty()) \
		return false; \
		\
		internal_signal::parameters[1] = static_cast<const void*>(&param); \
		return Signal::emit(widget); \
	} \
	\
	private: \
		/************************************************************************************************************************ \
		 * @internal
		 * @brief Checks whether the unbound parameters match with this signal
		 * @return The index in the parameters list where the parameters will be stored
		 ************************************************************************************************************************/ \
		unsigned int validateTypes(std::initializer_list<std::type_index> unboundParameters) const override; \
};

	EXTRA_SIGNAL_VALUE_DECLARATION(EditorAttack, std::shared_ptr<EditorAttack>)
	EXTRA_SIGNAL_VALUE_DECLARATION(EditorMovablePoint, std::shared_ptr<EditorMovablePoint>)
	EXTRA_SIGNAL_VALUE_DECLARATION(SoundSettings, SoundSettings)
	EXTRA_SIGNAL_VALUE_DECLARATION(Animatable, Animatable)

	class SignalTwoInts : public Signal {
		public:

			using DelegateRange = std::function<void(int, int)>;
			using DelegateRangeEx = std::function<void(std::shared_ptr<Widget>, const std::string&, int, int)>;
			using Signal::connect;

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/// @brief Constructor
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			SignalTwoInts(std::string&& name) :
				Signal{ std::move(name), 2 } {
			}


			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/// @brief Connects a signal handler that will be called when this signal is emitted
			///
			/// @param handler  Callback function that is given a child window as argument
			///
			/// @return Unique id of the connection
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			unsigned int connect(const DelegateRange& handler);


			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/// @brief Connects a signal handler that will be called when this signal is emitted
			///
			/// @param handler  Callback function that is given a pointer to the widget, the name of the signal and a child window as arguments
			///
			/// @return Unique id of the connection
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			unsigned int connect(const DelegateRangeEx& handler);


			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/// @internal
			/// @brief Call all connected signal handlers
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			bool emit(const Widget* widget, int value1, int value2);


		private:

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/// @internal
			/// @brief Checks whether the unbound parameters match with this signal
			/// @return The index in the parameters list where the parameters will be stored
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			unsigned int validateTypes(std::initializer_list<std::type_index> unboundParameters) const override;


			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	};
}
