## Simple header-only C++ objects for NASDAQ ITCH 5.0 protocol

Simply take a peek in the single `itch.h` header file for details.

There is a constructor if you want to create a blank object, and a constructor where you pass in a char pointer
that contains the record.

There are getters and setters for int and string (more soon). Below is a brief example. See `test` directory
for more examples:

```
    itch::system_event msg;
    msg.set_int(msg.STOCK_LOCATE, 1);
    msg.set_int(msg.TRACKING_NUMBER, 2);
    msg.set_int(msg.TIMESTAMP, 6);
    msg.set_string(msg.EVENT_CODE, "O");
    int timestamp = msg.get_int(msg.TIMESTAMP); // should equal 6
```

TODO:
- Test each object for their length
- Test each object for their prefix
- Test each field of each object for setting and retrieval
