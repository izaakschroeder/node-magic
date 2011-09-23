
#include <cstring>

#include <v8.h>

#include <node.h>
#include <node_buffer.h>

#include <magic.h>

/*


*/

#define THROW_ERROR(TYPE, STR)                                          \
	return ThrowException(Exception::TYPE(String::New(STR)));

#define REQ_ARGS(N)                                                     \
  if (args.Length() < (N))                                              \
    return ThrowException(Exception::TypeError(                         \
                             String::New("Expected " #N "arguments"))); 

#define OPT_STR_ARG(I, VAR, DEFAULT)                                    \
  const char* VAR;                                                              \
  if (args.Length() <= (I)) {                                           \
    VAR = (DEFAULT);                                                    \
  } else if (args[I]->IsString()) {                                      \
    VAR = *String::Utf8Value(args[I]->ToString());                                        \
  } else {                                                              \
    return ThrowException(Exception::TypeError(                         \
              String::New("Argument " #I " must be a string"))); \
  }

#define REQ_STR_ARG(I, VAR)                                             \
  const char* VAR;														\
  if (args.Length() <= (I) || !args[I]->IsString())                     \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be a string")));    \
  String::Utf8Value _VAR(args[I]->ToString());		\
  VAR = *_VAR;

#define REQ_INT_ARG(I, VAR)                                             \
  int VAR;                                                              \
  if (args.Length() <= (I) || !args[I]->IsInt32())                      \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be an integer")));  \
  VAR = args[I]->Int32Value();


#define REQ_BUF_ARG(I, VARBLOB, VARLEN)                                             \
  const char* VARBLOB;													\
  size_t VARLEN;	                                                    \
  if (args.Length() <= (I) || (!args[I]->IsString() && !Buffer::HasInstance(args[I])))                      \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be an buffer")));  \
 if (args[I]->IsString()) { \
	String::AsciiValue string(args[I]->ToString()); \
	length = string.length(); \
	blob = *string; \
} else if (Buffer::HasInstance(args[I])) { \
	Local<Object> bufferIn=args[I]->ToObject(); \
	length = Buffer::Length(bufferIn); \
	blob = Buffer::Data(bufferIn); \
}


                  
#define OPT_INT_ARG(I, VAR, DEFAULT)                                    \
  int VAR;                                                              \
  if (args.Length() <= (I)) {                                           \
    VAR = (DEFAULT);                                                    \
  } else if (args[I]->IsInt32()) {                                      \
    VAR = args[I]->Int32Value();                                        \
  } else {                                                              \
    return ThrowException(Exception::TypeError(                         \
              String::New("Argument " #I " must be an integer"))); \
  }

                  



using namespace node;
using namespace v8;



class Magic : ObjectWrap {
public:
	
	static Persistent<FunctionTemplate> constructorTemplate;
	
	magic_t magic;
	int localFlags;
	
	
	Magic(magic_t m) : ObjectWrap(), magic(m) {
		
	}
	
	~Magic() {
		if (magic != NULL)
			magic_close(magic);
	}
	
	operator magic_t () const {
		return magic;
	}
	
	
	
	static void Init(Handle<Object> target) {
		
		
		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		
		//Create a new persistent function template based around "create"; this
		//template is used as the prototype for making new instances of the object
		constructorTemplate = Persistent<FunctionTemplate>::New(t);
				
		//This object has one internal field (i.e. a field hidden from javascript);
		//This field is used to store a pointer to the image class
		constructorTemplate->InstanceTemplate()->SetInternalFieldCount(1);
		
		//Give the class a name
		constructorTemplate->SetClassName(String::NewSymbol("Magic"));
		
		//All the methods for this class
		NODE_SET_PROTOTYPE_METHOD(t, "file", file);
		NODE_SET_PROTOTYPE_METHOD(t, "buffer", buffer);
		NODE_SET_PROTOTYPE_METHOD(t, "load", load);
		NODE_SET_PROTOTYPE_METHOD(t, "flags", flags);


	}
	
	static Handle<Value> New(const Arguments &args) {
		HandleScope scope;
		Magic* magic = new Magic(NULL);
		magic->Wrap(args.This());
		return args.This();
	}
	
	static Handle<Value> file(const Arguments &args) {
		REQ_STR_ARG(0, file)
		Magic* magic = ObjectWrap::Unwrap<Magic>(args.This());
		const char* output = magic_file(*magic, file);
		if (output == NULL)
			return ThrowException(String::New(magic_error(*magic)));
		return String::New(output);
	}
	
	static Handle<Value> buffer(const Arguments &args) {
		REQ_BUF_ARG(0, blob, length)
		Magic* magic = ObjectWrap::Unwrap<Magic>(args.This());
		const char* output = magic_buffer(*magic, blob, length);
		if (output == NULL)
			return ThrowException(String::New(magic_error(*magic)));
		return String::New(output);
	}
	
	static Handle<Value> load(const Arguments &args) {
		OPT_STR_ARG(0, file, NULL)
		Magic* magic = ObjectWrap::Unwrap<Magic>(args.This());
		if ( 0 != magic_load(*magic, file) )
			return ThrowException(String::New(magic_error(*magic)));
		return args.This();
	}
	
	static Handle<Value> flags(const Arguments &args) {
		OPT_INT_ARG(0, flags, -1);
		Magic* magic = ObjectWrap::Unwrap<Magic>(args.This());
		if (flags == -1) {
			return Integer::New(magic->localFlags);
		} else {
			int res = magic_setflags(*magic, flags);
			magic->localFlags = flags;
			return args.This();
		}
	}

	static Handle<Value> create(const Arguments &args) {
		
		HandleScope scope;
		
		
		magic_t m = NULL;
		int flags = 0;
		const char* file = NULL;
		
		if (args.Length() > 2)
			return  ThrowException(Exception::TypeError(String::New("Arguments must be flags/file path")));
		
		for(int i = 0; i < args.Length(); ++i)
			if (args[i]->IsInt32())
				flags = args[i]->Int32Value();
			else if (args[i]->IsString())
				file = *String::Utf8Value(args[i]->ToString());
			else 
				return  ThrowException(Exception::TypeError(String::New("Arguments must be flags/file path")));
			
		
		
		
		if (NULL == (m = magic_open(flags))) {
			return ThrowException(String::New("Unable to open magic!"));
		}
		
		if (0 != magic_load(m, file)) {
			magic_close(m);
			m = NULL;
			ThrowException(String::New(magic_error(m)));
		}
		
		Local<Object> object = constructorTemplate->GetFunction()->NewInstance();
		Magic *magic = ObjectWrap::Unwrap<Magic>(object);
		magic->magic = m;
		magic->localFlags = flags;
		
		return scope.Close(object);
	}
	
	
};

Persistent<FunctionTemplate> Magic::constructorTemplate;

extern "C" {
	static void init (Handle<Object> target)
	{
				
		//http://linux.die.net/man/3/libmagic
		NODE_DEFINE_CONSTANT(target, MAGIC_SYMLINK);
		NODE_DEFINE_CONSTANT(target, MAGIC_COMPRESS);
		NODE_DEFINE_CONSTANT(target, MAGIC_DEVICES);
		NODE_DEFINE_CONSTANT(target, MAGIC_MIME);
		NODE_DEFINE_CONSTANT(target, MAGIC_CONTINUE);
		NODE_DEFINE_CONSTANT(target, MAGIC_PRESERVE_ATIME);
		NODE_DEFINE_CONSTANT(target, MAGIC_RAW);
		
		NODE_SET_METHOD(target, "create", Magic::create);
		
		Magic::Init(target);
	}

	NODE_MODULE(magic, init)
}