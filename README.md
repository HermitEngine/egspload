# egspload 0.1.0
This is a small non-fuss utility to easily serialize C structs with as little code
as possible. This includes deeply nested structs.

## Who should use this?
If you want to serialize your own structs and classes, I highly recommend that you use
one of Protocol Buffers, Flatbuffers or Cap'n Protocol. They each have different performance
profiles but largely achieve the same end result.

egspload should be used when you want to serialize structs from third-party libraries without
making changes to their code. It might also appeal if you are a C afficionado and the new C++
paradigms like templates, enum classes and lambda functions offend you. There are some major
tradeoffs you make for the simplicity.

* There is no cross-language support. Even C++ is only partially supported, by the sole virtue 
of it being a superset of C.

* There is no versioning or backward compatibility of data schemas

* There are no optional fields

* It is considerably less tried and tested

In return, what you gain is:

* No dynamic memory allocation. You provide the single memory buffer and everything is fit into it.
If you are particular about alignment, that too is configurable.

* The code base is small and easy to eyeball for the experienced programmer. As a result, it is 
easy to make changes to support your particular use case.

* There is less setup to do. Specify the structs you wish to serialize and you are good to go.
Strings, enums, and nested structs are handled automagically.

* The data is serialized in chunks, making it easy to double-buffer and/or split into packets.

## Quickstart
_Actually it is the only start. It is that simple!_

### Step 1: Create the definitions file
This file describes which fields from the structs you wish to serialize. Refer to
egsptest.egsp for an example. The basic data types include uint64_t, int64_t,
uint32_t, int32_t, uint16_t, int16_t, uint8_t, int8_t and string. Any struct you
declare in this file can be used as a data type for any other struct.

A \* after the data type means that it is a pointer the referenced type.

A % after the data type means that it is an enumeration value. All enums are stored
as int32_t and this is reflected in Json as opposed to the actual enum label.

A \[sometext\] after the variable name indicates it is an array of the referenced
type of size indicated by the "sometext" variable in the same struct.

### Step 2: Feed the file to egsploader.exe
The syntax is: egsploader firstfile, secondfile, thirdfile...

This will produce a egspload.h file which you can then \#include in your code.
Also be sure to link egspload.lib (or include egsplib.c in your project) and have 
egsplib.h in your include path.

### Step 3: Call the relevant function
There are only 4 per struct. It is easy to tell which to call because they are the only functions
that are not prefixed by an underscore and contain the name of your struct. (We will use TestStruct
as an example)

 ```c
// example flush func that writes to a file, single buffered.
uint8_t* FlushFunc(size_t size)
{
	fwrite(buffer, 1, size, s_pFile);
	return buffer; // buffer must be at least "size" big
}

EgspResult EgspSaveTestStruct(EgspFunc pFlushFunc, TestStruct* pVal, size_t* pHeapRequired);
EgspResult EgspPrintTestStruct(EgspFunc pFlushFunc, TestStruct* pVal, size_t* pHeapRequired);
```

These two functions serialize TestStruct. EgspSaveTestStruct writes to binary and EgspPrintTestStruct
writes to Json. The parameters are as follows:

* pFlushFunc: A function pointer that gets called every time more buffer space is needed to write to. The
size parameter indicates the size of data that has actually been written. Most handy for the incomplete last
block. It returns the buffer egspload will write to next.

* pVal: A pointer to the top-level struct you wish to serialize

* pHeapRequired: The amount of memory you will need to allocate all of the nested structs and strings. 
It is recommended you store this number for when you want to reload it. (e.g. at the beginning
of the file you are writing to or in the first packet sent over the network)

```c
EgspResult EgspLoadTestStruct(EgspFunc pLoadFunc, TestStruct* pVal, void* pHeap, size_t heapSize);
EgspResult EgspReadTestStruct(EgspFunc pLoadFunc, TestStruct* pVal, void* pHeap, size_t heapSize);
```
These two functions deserialize TestStruct. EgspLoadTestStruct loads from binary and EgspReadTestStruct
loads from Json. The parameters are as follows:

* pLoadFunc: A function pointer that gets called every time egspload has run out of data to parse.

* pVal: A pointer to the structure that is to be loaded with the read data.

* pHeap: A memory buffer that __you__ allocate, which is hopefully of the size that was indicated by the
preceding Save/Print functions or greater. __You__ can free this memory at your own leisure when you are done
with the struct.

* heapSize: The size of the memory buffer pointed at by pHeap. If it is too small, the load will fail.

### Step 4: Profit!
Note the lack of ???. I have a strong dislike for ???.

## Gotchas
_This is C so of course there are gotchas._

1. The Json reader is not real. It disregards the "variable names", and instead reads the data in sequence.
It is also a fair bit slower than binary. Its main purpose is to allow development-time visualization and tweaking 
of the data in your favorite Json editor. (Real programmers use Vim!) The whole Json functionality can be 
excluded via the CMake configuration.

2. egspload fills up the supplied memory buffer from the end of the buffer instead of the start. If you were
thinking of using a big buffer for egspload as well as additional custom data, take note.

3. To maintain cross functionality with C++ classes the "struct" keyword is not included in any of the generated
code. If you use a C++ compiler, this will not apply. Otherwise all structs should be defined as follows:
	```c
	typedef struct {
		int dummy;
	} TestStruct;
	```

4. If you serialize an array, be sure that the member variable containing the array size is ahead of it in the schema
file. There is no reason to follow the order of the struct other than data cache optimization.

5. Unicode is not supported. Strings are strictly null-terminated C strings.

6. egspload needs to know about your structs. #include the struct definitions before you #include "egspload.h".

7. You cannot have a list of pointers. The list (\[\]) operator supersedes the pointer(\*) operator.

## FAQ
_To be honest, nobody has actually asked any of these questions. I just thought it would be handy._

### What does egspload stand for?
Eugene Goh's (that's me!) Struct Parser/Loader.

### Pun intended?
Most forcedly.

### Why do you not directly use the base types like int, long, char etc.
In data storage, it is good to be explicit about the size of your types in storage. It is hoped that as a future feature,
a 64 bit integer can be forced into an 8 bit one, given you know the bounds wouldn't be exceeded. In the current version,
this is not possible to do safely. If you use the wrong type, the compiler should throw up warnings for the generated code.

### How efficient is the binary format in terms of size?
The binary format is simply the data written as is. Strings are stored as a 32 bit unsigned representing the length
of the string followed by actual characters. If it pleases you, you may pass the binary stream through your preferred
bit packer or compression library.

### Does egspload handle endian-ness?
Yes. egpspload is endian-agnostic. It cares not one whit what the native endian format is. For full disclosure,
numbers are stored big-endian in the binary format should you want to write your own parser.

### Is egspload safe to use in production?
By itself, there are no weaknesses I am aware of other than:

* If you serialize a struct with pointers pointing at garbage, garbage will ensue. I respect you as a C programmer, and
spend zero CPU cycles second-guessing the validity of your data.
* The code generator has a maximum string buffer of 4MB per struct (1MB per each type of function) that may be overrun in
the case of very very large structs.
* The numbers in the Json loader/reader are serialized in and out of a 256 byte string buffer with no bounds check. 
But as mentioned, Json is not for production.

That being said, the codebase is fairly new and has not undergone any security audits, so bugs are possible. If you
do discover any bugs or vulnerabilities, please let me know by raising an issue on Github or even better, submitting
a patch!

### How can I ensure my structs are optimally memory aligned?
By default, egspload adheres to 16-bit alignment which is the minimum necessary for 64 bit systems. You may freely change
this by calling EgspSetAlignBytes() prior to operating on any data. EgspAlignBytes() will return the current alignment in
bytes. Note that if you have an array of structs, egspload only guarantees alignment on the first one. It is up to you to
make sure your structs are correctly padded.

### But does it scale?
Yes it scales! By using memory blocks instead of a flat buffer, you can work on arbitrarily large data sets while only taking
up (by default) 4KB of memory. Of course, your structs themselves would be pretty humungous for that to matter.

### Can I use a custom memory allocator?
Use whatever you want dude! egspload itself does no heap allocations. Instead, it asks you for the memory which you can allocate
anyway that you want, provided that the memory is continguous and that you give it a NULL pointer if you are unable to provide.

### How big should my memory block be? Can I change this number?
By default, it is 4KB. You can change it by calling EgspSetBlockSize(). EgspBlockSize() will return the currently set 
block size. At certain levels of insanity, you may change the block size dynamically each time your Flush/Load function is called.
Oh, and just to put it out there, 4KB is by no means a good "rule of the thumb" or a bad one either. The optimal size will depend
on your implemntation. Profile! Profile! Profile!

### I have a need for speed. How do I go faster?
If you are a threading guru, you can double-buffer. In your Flush/Load function, return an alternating buffer, and have another
thread handle whatever it is you are doing with the data. If you truly want to zoom, you can also operate on different structs
concurrently, as long as each of the threads have their own buffer. The only global data that is shared is the byte alignment
and the block size, so stay away from the insane option or make them thread-safe if you want to pursue this.

### Can it work with std::string?
No. It is not hard to add, but this would cease to be a C library.

### Well if that is easy, supporting Unicode should be too right?
Correct. I do not have the need for it as yet, and am unlikely to with my current project. If you do want to add it, I would 
suggest a new data type.

### Does egspload handle versioning of data?
No. Though largely inspired by protocol buffers, I have ditched a whole lot of cruft including optional fields and backward
compatibility in favor of simplicity. In order to bring your data up to date, the best approach would be to generate 2 egspload.h
files. #include them from different .c files and use one to load data into your structs and the other to save them in the new
format.

### What languages is egspload supported in?
Just C. To a limited extent, C++. If you want a cross-language solution, I recommend you look at Protocol Buffers, Cap'n Proto or
Flatbuffers and pick your poison. If you do port egspload to another language, I would be more than happy to include a mention in 
this section. Just add a pull-request for README.md.

### What compilers has egspload been tested on?
Visual Studio 2017 and gcc 5.4.0.

### I want to contribute a bugfix/patch/enhancement. What should I do?
First, be sure all your code is C compliant. By that, I mean it compiles on the abovementioned compilers. Ensure there are no
extra library dependencies. And keep it simple. Submit a pull request. If I like it, I will merge it. If not, I will suggest
changes or simply forking. Oh, and if you were wondering, I am a tabby cat. Death to spacers!

### So why did you write this library in the first place?
Vulkan. Seriously, 90% of code dealing with that API is all about populating structs. Something had to be done!