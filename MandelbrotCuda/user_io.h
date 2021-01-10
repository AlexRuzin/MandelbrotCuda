#pragma once

namespace user_io {
	typedef enum {
		IO_MODE_MANUAL,
		IO_MODE_AUTO
	} IO_MODE;

	class io {
	private:
		IO_MODE mode;

	public:
		io(void) :
			mode(IO_MODE_AUTO)
		{

		}

		~io(void)
		{

		}

		// Sets control to automatic or manual
		void set_control_mode(IO_MODE mode) 
		{
			mode = mode;
		}

		IO_MODE get_control_mode(void) const
		{
			return this->mode;
		}
	};
}