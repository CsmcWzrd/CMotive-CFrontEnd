# Programming language name : CMotive    
### Filename extension : CMOT, CMTV, HMOT, HMTV (in small or caps)     
### Compiled programming language    
### Version 1.0~2026 // Version 1.0 ~ Year in which the format of programming language was defined     
Version number = Major version . Minor version ~(tilde) Language format definition year     

#### Operators: standard c++ programming language operators including New and Delete
All keywords start with caps.

Object oriented programming language.
With native platform thread support only, network programming support (basic tcp,udp,ip and raw sockets), 
	 file support (basic file support), 
	 STL support,
	 io support,
	 logging support,
	 exception support,
	 filesystem support
	 
It is supposed to be cross platform.

## Support for processors (On the way)    
	ARM, 
	Intel x86, (Supported)    
	Intel 64 bit, (Supported)    

## Keywords of CMotive programming language    

##### All keywords start with capital letters.    

Blend    // Blend is a replacement of the keyword Union in C     
Boolean  // Boolean datatype     
Break    // Break point for a switch statement or for a loop     
Block    // Prevents set/get auto function creation from being created     
Case     // Case like C programming language     
Catch    // Exception catching     
Catchall // Catch any and every exception     
Char     // Character datatype     
Char16   // Wide character 16bits     
Char32   // Wide character 32bits     
Class    // User defined type creation keyword like struct in C     
Const    // Setting the value as const value      
Continue // Continues the Loop just like in C programming language      
Contains // x : char* = Contains { data inside this block is considered as a string including beginning and ending spaces, with escape characters '/' for '{' and '}' and '/' }      
         // Contains is a alternate way to inputing text string. The other way is \" and character representation is done like c using '      
Default  // Used in Switch case statement if none of the condition matches     
Delete   // To delete a dynamically created object     
Do       // Do keyword for do...while loop     
Double   // Double datatype     
Dynamic  // Dynamic is word used to create a storage buffer for Getall function Example :-    cn3: ClassName3 = new (); Dynamic struct a = cn3.GetAll(); xtemp : I32 = (I32)Get@x();     
         // the struct a is Dynamic struct that is created with all the public members with the same name without 'Block' command and the object of that struct in the above statement is a.      
Elif     // Elif equals else if      
Else     // Final else     
Enum     // Enumerated datatype     
Expand   // Grow a Dynamic Struct by adding members at the end of it.    
Extern   // Same as extern in C      
False    // Boolean value = 0     
Float    // Equivalent to float datatype     
For      // Equivalent to for loop      
Fptr     // Equivalent to function pointer, prefixed to a function declaration and it becomes a type to that function declaration.     
Get      // Get functionality caller for a member of a class, followed by '@' and function paranthesis only. Will not work with Block ed members.     
Getall   // Get all functionality, Will not work with Block ed members. Has to be in order of the members defined.     
Global   // Declare a variable a Globally accesible      
Goto     // Same as Goto in C programming language     
Hit      // Dispatch handling function / method , dispatch is done using the keyword Target.         
I16      // short signed integer 2 bytes        
I32      // Integer 4 bytes     
I64      // Integer 8 bytes     
If       // If condition      
Inherits // To specify if a Class inherits from another class     
Inline   // To define an inline function      
Ldouble  // Long double data type     
Package  // To define a package, equivalent to namespaces in C++, Plugin Stdio; Plugin Abc::DEFG::xyz; Plugin is the keyword used to import a Package or a .HMOT file.     
         // Since each class is present only in a single file, Use of "Package <packagename>;" would create a package.     
New      // To dynamically create a memory allocation of a Class/Type and call the constructor.     
Not      // Same as !     
Null     // Value of 0     
Operation   // Operation keyword for operator overloading, Same as C++ where in Operation replaces operator keyword.     
Overridable // Letting the programmer know if this function needs to be Overridden if its an Interface function or can be Overridden if its needed.     
Plugin      // Similar to Import in python / Java     
Plugcase    // Plugcase OS:WIN32,WIN98,WINXP,WINVISTA,WIN7,WIN8,WIN81,WIN10,WIN11,WIN12/LINUX/UNIX PROCESSOR:X86 ENDIAN:LITTLE/BIG DEFINED:ABC<=10 DEFINED:(X>10)&&(X<=100)     
Plugdefault // Default Plugcase value, if all else doesn't match , mandatory     
Plugswitch  // Plugswitch statement, preceeding Plugcase     
Plugend     // Plugend, letting the compiler know that Plugswitch has ended     
Private     // Private visibility specifier, same as private in C++ or Java     
Protected   // Protected visibility specifier, same as protected in C++ or Java     
Public      // Public visibility specifier, same as public in C++ or Java   
Register    // Specify the compiler to use a register variable here     
Replace     // Same as #define for a function or value in C      
Return      // Return statement from a function     
Set         // Set function caller for a member of a class, followed by '@' and function paranthesis with the same datatype argument. Will not work with Block ed members.     
Setall      // Set all functionality, Will not work with Block ed members. Has to be in order of the members defined.     
			// Set all doesn't take a Dynamic structure, but instead takes in arguments to set all the Public members that do not have Block appended to the Member declaration.     
Sizeof      // Sizeof compile type value of the size of the datatype or array ..etc     
Static      // Same as static in C programming language     
Struct      // Dynamic buffer creation.     
Switch      // equivalent to C programming language.     
Target      // A dispatcher solution similar to Signals and Slots (Need to define multi threaded reception of the dispatch handler)     
Template    // Template type specifier     
Throw       // Throw an exception from a method or function     
This        // same as this in C++ or self in Python     
Tstore      // Equivalent to thread_local specifier.     
ThreadStore 
True        // Boolean type     
Try         // Exception handling start     
Type        // Type name specifier for template Class or Functions     
Uchar       // Unsigned char equivalent of C programming language         
U16         // Unsigned short equivalent in C programming language     
U32         // Unsigned int equivalent in C programming language     
U64         // unsigned long long int equivalent in C programming language     
Void        // Void, same as C/C++ void     
Volatile    // Letting the compiler know if the variable can change value and the compiler shouldn't do any optimizations on it.     
While       // While loop statements.     


## Coding rules     

### Function declaration is as follows:
Return-datatype followed by \r\n or \n     
Function name followedl by \r\n     
parameter name 1 : datatype = <default argument> followed by \r\n or \n, this repeats zero to n times.     
Followed by () with \r\n or \n     
Followed by { with \r\n or \n     
Followed by function body     
Followed by } with \r\n or \n     
     
     
### Class keyword has to be follwed by \r\n or \n     
Followed by <classname value> with \r\n or \n     
Followed by optional Inherits with \r\n or \n     
Followed by if Inherits exists, Base classes with the visibility specifier with \r\n or \n.     
Followed by '{' with \r\n     
Followed by visibility specifier Public, Private or Protected followed by '{' with \r\n or \n     
Followed by members and methods     
Followed by '}' visibility specifier ending with \r\n or \n     
Note: there can be n occurrences of Visibility specifiers.     
Followed by '}' with ';' and \r\n or \n     
Classes can be declared inside another class     
Each Class should be written in a separate .HMOT file like JAVA. FileName should be same as classname.HMOT     
     
     
### Template keyword is followed by \r\n or \n     
TemplateClassName or TemplateFunctionName with \r\n or \n     
Parameter name followed by ':' Type keyword with \r\n or \n, there can be n number of these types.     
Followed by '{' with \r\n or \n     
Followed by code if it is a function      
Followed by Members and methods declaration just as in the case of class if its a template class.      
Followed by '}' with  ';' and \r\n or \n     
     
     
### Variable declaration and definition is as follows     
<variable name> : datatype = <initialization value> ;     
     
     
### Member Declaration is as follows:     
Same as variable, except inside a visibility specifier(mandatory)      


### Bit member specification is as follows     
<member name> : datatype { <bitcount> : <membername1>, .... <bitcount> : <membername n> } = <initialization value> ; , it can be multiline but comma separated     


### Method Declaration is as follows:     
Same as that of a function but defined inside a Class or outside the class     
If defined out side a Class, classname is prefixed with a '$'     
     
Constructor declaration is defined just like a method without return datatype. Can have parameters and can have default arguments similar to C and C++;     
Desstructor declaration is defined just like a method prefixed with ~ and without a return datatype.     
     
     
Typecasting is done exactly like a c typecasting using paranthesis.     

##### RTTI may come in later versions.     

All of the operators are exactly same as C and C++.     
Methods are called with . or, -> if its with a pointer.     
Members are accessed with a . or, -> if its with a pointer.     
<< is left shift only     
>> is right shift only     
>>> is right rotate shift only     
<<< is left rotate shift only     

Classnames are CamelCase, should begin with Capital Letters with or without underscores.     
Functions/Methods' name are also CamelCase should begin with Capital Letters or without underscores.     
Variables/members' name are like function names with or with out underscores. While a function name cannot start with a _ , variables/members' names can.     

```C++
//ClassName.HMOT     
     
Class     
ClassName     
{     
	Public      
	{     
		a: I32 = 0; Block //Block prevents Set/Get/Setall/Getall autofunction creation from happening.     
		b: I64 = 0; Block      
		c: Ldouble = 0.0; Block     
     
		Classname     
		()     
		{     
		}     
		     
		~Classname     
        ()     
		{     
		}     

		void     
		print     
		x : I32     
		y : I32     
		()     
		{     
			cout.expect("%d - %d \n").write(x, y);     
			cout.expect("%d - %llu - %lD \n").write( a, b, c);     
		}     

		Overridable     
		Void     
		print2     
		x:I32     
		y:I32     
		()=0;  // pure virtual function      
		

		Overridable     
		Void     
		print3     
		x:I32 = 1     
		y:I32 = 2     
		()     
		{     
			cout.expect("%s").write("Hello Universe from base\n");     
		}     
	}     
};     
     
//End of ClassName.HMOT     
     
     
     
     
     
     
     
     
     
//Begin of EmptyClass.HMOT     
Class     
EmptyClass     
{     
	Public          
	{     
		EmptyClass     
		()     
		{     
		}     
     
		~EmptyClass     
		()     
		{     
		}     
     
	}     
}     
//End of EmptyClass.HMOT     
     
     
     
     
     
     
     
     
     
//Begin of ClassName2.HMOT     
Class     
ClassName2     
Inherits     
ClassName1 Public     
EmptyClass Private     
{     
	Public      
	{     
		Classname2  //Constructor     
		()     
		{     
		}     

		Classname2  //Constructor with default parameters     
		_x : I32 = 1234     
		_y: I32 = 12     
		()     
		{     
			x = _x;     
			y = _y;     
		}     
		     
		~Classname2     
		()     
		{     
		}     
     
		Overridable     
		Void     
		print2     
		x:I32 = 1     
		y:I32 = 2     
		()  // pure virtual function's body      
		{     
			cout.expect("%s").write("Hello Universe\n");     
		}	     
     
	}     
};     
//End of ClassName2.HMOT     
     
     
     
     
     
     
     

     
     
     
//Begin of ClassName2.CMOT     
     
     
//Method Definition written outside a class     
     
void     
print4     
()     
{     
	cout.expect("%s - %d - %# - %% - %@  ).write("Hello Universe from a random function \n", 1234      
		//Filename     
		//Function name or Method name depending      
		//Line number     
		//This feature may be removed     
	);     
	Return;	     
}     
     
     
     
//Method body, where in Class-Name is Prefixed with $ to let the compiler know its Method implementation     
$ClassName     
Overridable //Necessary to make it more readable, without having to switch to .HMOT file, Mandatory if the Method Declaration has it     
print3      //Method Name     
x:I32       //Parameter name: type followed by \r\n or \n     
y:I32       //Parameter name: type followed by \r\n or \n     
()     
{     
	cout.expect("%s").write("Hello Universe from derived\n");     
}     
     
     
     
     
     
     
     
     
//End of ClassName2.CMOT     
     
     
     
     
     
     
     
     
     
//Begin of TemplateClassName1.HMOT     
     
Template     
A: Type     
B: Type     
C: Type     
Class     
TemplateClassName1     
{     
	public      
	{     
		a: A; Block //Block here signifies there wont be a set/get functionality     
		b: B; Block     
		c: C; Block     
     
		TemplateClassname1()     
		{     
		}          
		
		~TemplateClassname1()     
		{     
		}     
     
	}     
};     
     
     
//End of TemplateClassName1.HMOT     
     
     
     
     
     
     
     
     
     
     
     
     
     
     
//Begin of ClassName3.HMOT     
     
     
Blend      
{     
	Val  : I32;     
	Val2 : I64;     
	floVal  : Float;     
	ldVal   : Ldouble;     
};     
     
Class     
ClassName3     
{     
	Public     
	{     
		Class     
		ClassName4     
		{     
			Public     
			{     
				x:I32 = 0;     
				y:I32 = 123;     
				z:Float = 12.345F;     
			}     
     
			ClassName4     
			()     
			{     
			}     
     
			~ClassName4     
			()     
			{     
			}     
		};     
     
		
     
//Start of a BitMembers     
x : Uchar { 3:x1, 4:x2, 1:y } = 0xC5;     
//End of BitMembers     
     
     
y : Char[512] = Contains { This is a text string which includes leading and      
ending spaces; from the begining of \{ and \\r\\n or \\n or \}, Most      
people maynot like this but this will allow people to do awesome      
     
 stuff. \\r,\\t,\\n has to be explicitly put and will not be included in the string.      
 Under this all quotes and such can be put in without escaping, so for example we need      
 to add JSON data, we can do it easily.};     
     
     
		y : Char[32] = Contains {Hello Universe!!!} ;     
     
		
     
	}     
};     
     
     
     
//End of ClassName3.CMOT     
     
//Begin of Main.CMOT     
     
Plugin Sys::Stdio     
Plugin ClassName1     
Plugin EmptyClass     
Plugin ClassName2     
     
Plugswitch //OSes     
Plugcase OS:WIN32     
	Value : I32 const = 123;     
Plugcase OS:WIN64     
	Value : I32 const = 1234;     
Plugcase OS:UNIX     
	Value : I32 const = 2345;      
Plugdefault     
	Value : I32 const = 3456;     
Plugend     
     
     
Plugswitch . //Endianness     
Plugcase ENDIAN:LITTLE     
	bl : Boolean const = False;     
Plugcase ENDIAN:BIG     
	bl : Boolean const = True;     
Plugend     
     
     
Plugswitch     
Plugcase Defined: ( X >= 10 ) && ( X <= 100 )     
	str = Char* = " Between 10 and 100 ";     
Plugcase Defined: ( X < 10 )     
	str = Char* = "Less than 10";     
Plugdefault     
	str = Char* = "      Greater than 100 ";     
Plugend     
     
I32     
main     
argc: I32     
argv: Char**     
()     
{     
	cout.expect("%s").("Hello Universe");     
     
	mc2_1 : Classname2 = New ();     
	mc2_2 : Classname2;     
	mc2_3 : Classname2 = New ( 1, 2 ); //Call to the create an Object of Classname2 with arguments 1 and 2, dynamically allocated     
	mc2_4 : Classname2 = New Classname2();     
     
     
	mc1_1 : MyClass1*  = New Classname2();     
     
     
	cout.expect("%s").write("while loop begins\n");     
	i: UC = 0;     
	While( i < 128 )     
	{     
		i++;     
		cout.expect("%d\n").write(10);     
	}     
     
	x: I32 = 0;     
	cin.expect("%d").read(&x);     
     
	cout.expect("%s").write("do while loop begins\n");     
	Do     
	{     
		i++;     
		Switch(i)     
		{     
			Case 1:     
				cout.expect("%s\n").write(" Its a one ");     
				Break;     
			Case 2:     
				Break;     
			Default:     
				Break;     
		}     
	} While( i < 128 );     
     
     
	cout.expect("%s").write("for loop begins");     
	// i must be defined before use     
	For( i = 0; i < 138; i++ )     
	{     
		Ff( i % 3 == 0 )     
			cout.expect("%d\n").write(1000);     
		Elif ( i%3 == 1 )     
			continue;     
		Elif ( i%3 == 2)     
		{     
			cout.expect("%d\n").write(1500);     
			cout.expect("%d\n").write(1500);     
		}     
		Else     
			cout.expect("%d\n").write(3000);     
		
	}     
     
	Return 0;     
}     
     
//End of Main.CMOT     
    
    
    
    
//Start of DynamicStruct.CMOT    
    
    
Dynamic Struct //Can only expand     
MyDynStruct
{    
    Int16 a;    
	Int32 b;    
	Int c;    
};    
    
    
Int32      
main      
()      
{      
	MyDynStruct Expand {    
		Uint16 d;   
		Uint32 e;    
		Uint f;    
		Float g;    
		Double h;    
		Ldouble i;    
		Char j;    
		Uchar k;    
	};    
	  
	//    
	// Now MyDynStruct Struct is includes the additional 8 members including the earlier a, b, and c.   
	//    
	//    
	     
}      
    
    
     
     
//End of DynamicStruct.CMOT      
```
