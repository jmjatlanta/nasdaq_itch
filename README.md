## Simple header-only C++ objects for NASDAQ ITCH 5.0 protocol

Simply take a peek in the single `itch.h` header file for details.

There is a constructor if you want to create a blank object, and a constructor where you pass in a char pointer
that contains the record.

There are getters and setters for int and string (more soon).

TODO:
- Test each object for their length
- Test each object for their prefix
- Test each field of each object for setting and retrieval
