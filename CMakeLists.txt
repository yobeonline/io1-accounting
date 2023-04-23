project(lib_accounting)

set_target(lib_accounting
	STATIC
		include/statement.hpp
		src/statement.cpp
		include/entry.hpp
		src/entry.cpp
		include/listing.hpp
		src/listing.cpp
		include/account.hpp
		src/account.cpp
		include/archived_listing.hpp
		src/archived_listing.cpp
		include/accounting_exception.hpp
		src/accounting_exception.cpp
		include/date_formatter.hpp
		src/date_formatter.cpp
		
	DEPENDS
		lib_money
		lib_exception
		lib_stream_utils
		Boost::boost
		Boost::filesystem
		Qt5::Core
		
	INCLUDE
		PUBLIC	include
)

set_target(test_lib_accounting
	TEST
		test/test_listing.cpp
		test/test_account.cpp
		test/test_entry.cpp
		test/test_statement.cpp		
		
	DEPENDS
		PRIVATE
			gtest_main
			lib_accounting
)
