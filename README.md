# ArduinoJStream
A Arduino/C++ library for parsing Json Streams with minimal memory usage.

## Why Json Stream?
Some Web-API Json responses can be huge which makes it impractical to save them, especially on devices with small memory capacity (e.g. ESP8266).<br/>
This library tries to solve that issue by not saving the whole response and instead extracting only the important data.

## Example usage
Lets consider the Json String:
```
    {
        "books": [
            {
                "title": "The Hobbit",
                "author": "J. R. R. Tolkien"
            },
            {
                "title": "The NeverEnding Story",
                "author": "Michael Ende"
            }
        ]
    }
```

We want to known the author of the second book, For simplicity lets assume we already made a request to the API and now have a stream object `stream` as a response.

``` 
    char* authorOfSecondBook = malloc(20);

    JsonParser parser;
    parser.parse(stream);

    parser.enterObj(); // Enter root Object
    parser.findKey("books"); // Find "books" key
    parser.enterObj(); // Enter "books" array

    parser.nextVal(); // Skip to second array element
    parser.enterObj(); // Enter book object
    
    parser.findKey("author"); // Find the "author" key
    parser.readString(authorOfSecondBook) // Read the author from stream into authorOfSecondBook
```

Alternatively a path can used instead:
```
    char* authorOfSecondBook = malloc(20);

    JsonParser parser;
    parser.parse(stream);
    
    parser.enterObj() // Enter root object
    parser.find("books[1]/author") // Find the author of the second book
    parser.readString(authorOfSecondBook) // Read the author from stream into authorOfSecondBook
```