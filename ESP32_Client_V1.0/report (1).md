This document contains my review of your code with some general observations, hints about potential bugs and some specific suggestions. I do hope it is helpful.

You already know my disclaimer, but I think it always bears repeating: Some comments might sound harsh or mean. **Please know that that is not my intention!** I appreciate and admire everyone who is brave enough to show their code to others and believe that deserves a lot of respect. **I also believe it deserves to be taken seriously**, which is why I do not hold back and mention as many possible improvements and pitfalls as I can find in the  time allotted.

I begin with some repeated general comments and observations and try to answer the questions you posed. After that, I  will highlight a few specific issues and go on to add some minor nitpicks.

- [Repeat Mentions](#repeat-mentions)
- [Answers to your questions/requests](#answers-to-your-questions/requests)
- [Specific issues](#specific-issues)
- [Minor nitpicks](#minor-nitpicks)

## Repeat Mentions:

These are a few issues we already mentioned in the last review, but which are still present in parts of the current code.
I therefore thought it might be beneficial to put them in here as well :)

### Header Files:

	- You use and include some parts of the  *C standard library*, specifically *stdint.h*. That is perfectly fine and useful, but when including headers originating from the C standard library in C++ code, it is generally considered best practice to not include *X.h* directly, but instead use the *cX* variant. So, instead of *stdio.h*, prefer to include *cstdio* and so on(see, for instance, in the current draft standard: https://eel.is/c++draft/support.c.headers, "Source files that are not intended to also be valid ISO C should not use any of the C headers.")

	- All header files prevent multiple includes in the same translation unit using the non-standard - but almost universally supported - *#pragma once*. I'd personally prefer header guards, for reasons [outlined here](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rs-guards).

	- Some include pathes seem to be inconsistent. In Networks/NetworkUSB.h you include Network/Network.h, in the corresponding cpp file, you include NetworkUSB.h without the path prefix.

	- You sometimes include headers that are not needed/used in a particular file. For instance, ExampleConfig includes vector.

	- *In general, include files and how to structure them properly are a bit of pet peeve of mine, which is why, a while ago, I wrote a [blog post](https://codemetas.de/2020/07/15/A-plea-for-order.html) detailing my thoughts on the matter.*

### Enums and constants

	- It might be a good idea to mostly use enum classes/scoped enums instead of C-style enums and get some additional type safety. Additionally, the scoping allows for better names that do not have to repeat themselves too much. Normal enums are implicitly convertible from and to int and poison the global scope with the names of their enumerators. See here for additional details: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Renum-class

### Linking issues

	- static globals in header files do not do what you probably think they do. static in file scope(see https://en.cppreference.com/w/c/language/file_scope) declares internal *linkage* (see https://en.cppreference.com/w/cpp/language/storage_duration), meaning every cpp file that included the respective headers will have **their own copy** of those variables/functions. To have global shared variables instead, you have to either declare them extern in the header file and define them in **exactly one** cpp file or declare them as inline. The latter works with C++>=17.

## Answers to your questions/requests

### C++17 features used and how to downgrade to C++11
	- Before we started talking, the only C++17 features you used were inline variables and a constexpr function in interrupt timer.
	  You can "downgrade" by replacing the *constexpr* before the function with a simple *inline* and declaring the variables as extern instead (And defining them **in exactly one cpp file**).

	- Depending on if and how you implemented the additional improvements we discussed, you might now also have std::string_view and std::optional in your code.
	  Those are luckily pure library additions and you can find versions of them backported to C++11 or implement them yourself.
	  (for instance, [here is a string\_view](https://github.com/martinmoene/string-view-lite) and [here is an optional](https://github.com/TartanLlama/optional))

### Instrument Controller, Message Handler and Network:
	- As there only ever exists one of them in each compiled version, it might be possible to make all global singletons or use CRTP or similar to avoid/resolve the dependencies,
	  but I think the main problem here is not necessarily solved by some hacks to allow it but by a slight redesign.

	  I collected your 1. and 2. in one answer, because I think the problems stem from the same cause:
	  I think the relationship between and InstrumentController, Message Handler and Network and their respective responsibilities is not
	  well enough defined and separate.
	  
	  - What exactly is *the one problem* Message Handler solves?
	  - What exactly is *the one problem* Network solves?
	  - What exactly is *the one problem* InstrumentController solves?

	  Are they separate problems? Is one of them trying to do too much? Or another too little?
	  
	  Maybe some parts here cannot be decoupled enough to justify an existence as seperate entities?
	  Maybe some parts are trying to do too much and should be split up?
	  
	  Such questions usually can help guide a better design.
	  
	  I first had to talk to you about your concrete reasoning for the current design, so I could give a
	  concrete suggestion on how to improve it here.

	  The solution we came up with was to decouple them and **let main/loop mediate between the parts**.
	  
	  Instead of forwarding the received messages directly, **connection returns a std::optional<MidiMessage|array>** and
	  if that was not nullopt, **loop forwards it to MessageHandler**, which in turn can return an optional response message,
	  which loop can hand to the network.

### Constants/enums
	- I think that was better addressed in the specifics below. I do not believe there is a good general rule to follow
	  except for: Try to keep their scope as low as possible, use typesafe enum classes where possible and put them
	  somewhere an uninitiated user might suspect them to be.

### ExampleConfig defines
	- Contrary to my other below comments, I would advocate for *using* here, despite it being in a header. It is much less intrusive then define.
	- Regarding the other defines, see generic comments about static and global variables. I'd avocate for constexpr!

### Device String performance
	- Accessing std::string should be exactly equivalent in performance to std::string. the only thing that is potentially wasteful is a single heap allocation on creation/change. Which will likely not happen as your string is small and the small-string-optimization will be used. So I wouldn't worry about it.
	- Nonetheless - as we noticed in our call - the string here can cause some issues in its usage, as some parts assume it to be exactly 20 characters long and it is assigned to from a const char* that is not necessarily 0-terminated. The latter would cause *undefined behaviour*, which in practice for this case means it would add all following bytes to the string until it hits a zero in memory. That is not a problem of the string itself, however, but one of the usage and should be adjusted by changing the usage as we discussed in the call (and as explained again below).

### Device as namespace
	- I think that is a fine choice, *if* there can never be a second device, it would not benefit from being a class at all. Only thing to consider is static initialization order fiasco and stuff like that :)
	- If there ever could be a second device and it makes sense to reuse all the code, just with different variables, it might make sense to actually(surprisingly) make it (almost) a singleton: create a class and create one function to return *the one true instance*

### Device.h
	- I think that is a good enough design for now and other issues should take precedent.
	- For providing defaults, we disussed the idea of having an additional header file containing defaults in a special defaults namespace. This allows users to include it and selectively pull the required defaults into the global namespace with help of [using directives or declarations](https://en.cppreference.com/w/cpp/keyword/using).

### Thoughts on local storage in MessageHandler
	- I am not exactly a fan of having empty functions that should never be used in the #else case, but I think it is an acceptable solution for the problem it is trying to solve.

### Extra/Local storage
	- We identified two main issues here, the first and small one was how the static global is handled. It should - at least - be inline and better something like a Meyers Singleton.
	- The second issue was the potentially error prone, hard to read and slightly inefficient way results were handed by passing out pointers and sizes.
		+ This lead to the potential problem with Device::Name mentioned above (maybe missing zero termination).
		+ As a std::string was created in all cases anyway, this problem can be solved by simply constructing it inside the function and returning the result. It should look similar to the following:

			std::string LocalStorage::GetDeviceName()
			{
				std::string result(20);
			    err = ReadNvsBlob("Device_name", result.data(), 20);

				if (err == ESP_ERR_NVS_NOT_FOUND)
					return Device::Name;
			    return result;
			}

		+ The other functions could be changed simlarly to make them more resilient, for instance:

			struct write_result
			{
				esp_err_t error;
				std::vector<std::uint8_t> data;
			};
			
			write_result LocalStorage::WriteNvsBlob(const char *key, uint8_t arrayLength)
			{
				OpenNvs();
				std::vector<std::uint8_t> data(arrayLength);
				err = nvs_set_blob(handle, key, data.data(), arraySize);
				if (err == ESP_OK) err = nvs_commit(handle);

				nvs_close(handle);
				return {err,data};
			}

## Specific issues

### AddrLED:
	- You use static inside a member function, but you use it to store part of the state of *this object*. This will break with multiple independent instances!
	- LED\_HUE\_METHOD is only used inside the class, so it could be moved into its scope (and get a slightly less verbose name, like "hue_method", for instance). Also, it could be an enum class to get a bit more type safety.
	- setLedOn/setLedOff read like getters/setters, but they do more than that (calling FastLED.show() in addition). I would suggest a rename to indicate this action. Maybe turnLedOn and turnLedOff?

### Device:
	- You likely want those const to be constexpr.
	- The "const char platform[]" strings would be slightly better as std::string_view. That is C++17 and above, however.
	- If you wish to keep the functions inside the header file, mark them as inline, not static! Otherwise you get muttiple independent copies.
	- What exactly is the reasoning behind (1<<0) instead of simply 1? If the <<0 is supposed to convey some special meaning of that position, I'd suggest using a named constant to indicate it? (in the call you fixed that quickly, as you already had a constant defined for it :))

	- If I understnad this piece of code correctly:

        for(uint8_t i = 0; i < 20; i++){
            if (Device::Name.size() >  i) deviceObj[10+i] = Device::Name[i];
            else deviceObj[10+i] = 0;
        }

	then you are copying up to 20 characters into deviceObj, starting at position 10? If so, I think this could be expressed more succinctly and readable, for instance:

		const auto characters_to_copy = std::min(20u,Device::Name.size());
		std::copy_n(Device::Name.begin(),characters_to_copy,&deviceObj[10]);
		std::fill_n(&deviceObj[10+characters_to_copy],20-characters_to_copy,0);

	or alternatively with two simple loops expressing the same.

### Constants
	- I do not like this file particularly much. Aside from the const and global issues, it collects a lot
	  of largely unrelated constants together in one file. How would I - as a user of your code - know which
	  constants appear in constants.h and which appear in the respective files corresponding to the component I am interested in?
	  I think it should be split up, constants for instrument type should be in a different file from those
	  describing a DistributionMethod and so on.

### Distributor
	- nextInstrument is rather large and therefore hard to follow. I think it would benefit from being split up a little
	- Seeing this:
	 
		void Distributor::controlChangeEvent(uint8_t Controller, uint8_t Value)
		{
		    //Implemented in MessageHandler
		}
		
	  I was wondering why this member function exists?
	  In the call you mentioned it was meant as documentation as it was the only api function of a group that was not implemented here.
	  We resolved to remove it and instead add a corresponding comment in the header file.

	- About this:

	    //Usefull Idea?
	    //memcpy(&distributorObj[4], &m_channels, 2);
	    //memcpy(&distributorObj[6], &m_instruments, 4);

	=> It is not really equivalent at the best of times (because of 7 relevant bits instead of 8) and platform dependent (due to endianess).

### PwmDriver:
	- All the file-static variables with m_ prefix are used like member variables, but only exist once for all instances. This will break if you ever have more than one PwmDriver, for whatever reason and is unintuitive due to the slightly misleading naming. I think either this should not be a class at all *or and I'd prefer that*, they should be member variables and you could use a Meyers Singleton to ensure everyone gets access to the one true instance([here is reasonably understandable explanation of what that is](https://laristra.github.io/flecsi/src/developer-guide/patterns/meyers_singleton.html))
	- If you opt to do that (i.e. make it a proper class type and instantiate it), you can no longer use a simple static member function for Tick, as you need access to the specific instance. This requires slight changes to your interruptTimer.
	  You have to use this function to register the interrupt:
	  
		    void timerAttachInterruptArg(hw_timer_t * timer, void (*userFunc)(void*), void * arg);

	  and pass the *this* pointer as the arg-void-pointer. The function you register has to look approximately like this:

		    static void interrupt(void* object_ptr)
		    {
			    static_cast<PwmDriver*>(object_ptr)->Tick();
		    }

	  You also should (regardless of which option you chose) probably ensure the interrupt is properly detached as well, i.e. call this function:
	  
		   void timerDetachInterrupt(hw_timer_t * timer);

	  in ~PwmDriver.
	  See [here](https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/timer.html) for documentation of the mentioned functions.
	  

	- The optimizations for Tick seems a bit suspect to me.

		#pragma GCC optimize("Ofast") // Required to unroll this loop, but useful to try to keep this speedy

	  Did you measure this? What optimization option are you compiling with usually? There are unrolling options that are more specific than Ofast, which contains a lot of other potential optimizations as well!
	  Would it be useful to compile with higher optimization levels in general.
	  Perfomance claims without measurements are sometimes dubious and can often lead to much less readable code for no measurable gain. So I'd suggest removing that and only adding it **if and when** it was determined to be needed.

	  If performance is crucial here and we try to not rely on the compiler doing exactly what we anticipate, the numActiveNotes condition (which seems loop invariant?) should most likely be lifted out of the loop.

	- m_currentState may benefit from being a bitset.

### MessageHandler
	- This class seems to be doing a bit too much and could probably be split up a little. Also see the notes above.

### Minor nitpicks
	- Why bool variable connected instead of simple return true;? (NetworkUSB)
	- nullptr is better than NULL (InterruptTimer::inialize)
	- Why private, public, private again if one of them happens to be empty anyway?
	- std::array really is better then C-Style arrays pretty much everywhere, but you use them inconsistently?
	- prefer constexpr over const
	- There are many comments that add no real value, repeating essentially the function name in other words and nothing else. That will just produce line noise and make the actual code less readable.
	  There is a famous quote by Bjarne Stroustrup: "The compiler does not read comments and neither do I!".
	  Comments should *only* be used to say that which cannot be expressed in the code itself. They can be used to document *usage* or *intent*, not to repeat what is already understood by anyone reasonably proficient in the language used. Otherwise, they are nothing more than future lies. 
	- prefer ++i in for loops instead of i++. There is no noticable difference on modern systems and compilers, but one does strictly more work and is more complicated than the other, so the "simpler" option should be prefered.
	- I'm not a fan of delay to hope everything gets done in a certain amount of time. Maybe there is some way to check whether we are ready instead of hoping for it?
	- Constructor initializer lists are the more idiomatic way to initialize member variables.
	- Try to avoid using in headers, as that would influence the meaning of code that inclues them.
	- (*something).whatever is equivalent to something->whatever but the latter is less noisy and more idiomatic.
	- The timer code may benefit from std::chrono.
	- Try to use return values instead of out-pointers. That is usually much less error prone and significantly more readable. If there is a buffer allocated and copied anyway (as was the case in large parts of your code), even if it would incur a copy(which most of the time it does not) it would not be more expensive(in terms of runtime and memory usage).
