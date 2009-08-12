/*
 * Copyright (c) 2009  Vegard Nossum
 *               2009  Tomasz Grabiec
 *
 * This file is released under the GPL version 2 with the following
 * clarification and special exception:
 *
 *     Linking this library statically or dynamically with other modules is
 *     making a combined work based on this library. Thus, the terms and
 *     conditions of the GNU General Public License cover the whole
 *     combination.
 *
 *     As a special exception, the copyright holders of this library give you
 *     permission to link this library with independent modules to produce an
 *     executable, regardless of the license terms of these independent
 *     modules, and to copy and distribute the resulting executable under terms
 *     of your choice, provided that you also meet, for each linked independent
 *     module, the terms and conditions of the license of that module. An
 *     independent module is a module which is not derived from or based on
 *     this library. If you modify this library, you may extend this exception
 *     to your version of the library, but you are not obligated to do so. If
 *     you do not wish to do so, delete this exception statement from your
 *     version.
 *
 * Please refer to the file LICENSE for details.
 */

#include <stdio.h>

#include "vm/die.h"
#include "vm/classloader.h"
#include "vm/natives.h"
#include "vm/preload.h"
#include "vm/class.h"

#include "jit/cu-mapping.h"

struct preload_entry {
	const char *name;
	struct vm_class **class;
};

struct vm_class *vm_java_lang_Object;
struct vm_class *vm_java_lang_Class;
struct vm_class *vm_java_lang_Cloneable;
struct vm_class *vm_java_lang_String;
struct vm_class *vm_array_of_java_lang_String;
struct vm_class *vm_java_lang_Throwable;
struct vm_class *vm_java_util_Properties;
struct vm_class *vm_java_lang_VMThrowable;
struct vm_class *vm_java_lang_StackTraceElement;
struct vm_class *vm_array_of_java_lang_StackTraceElement;
struct vm_class *vm_java_lang_Error;
struct vm_class *vm_java_lang_OutOfMemoryError;
struct vm_class *vm_java_lang_ArithmeticException;
struct vm_class *vm_java_lang_NullPointerException;
struct vm_class *vm_java_lang_NegativeArraySizeException;
struct vm_class *vm_java_lang_ClassCastException;
struct vm_class *vm_java_lang_ClassNotFoundException;
struct vm_class *vm_java_lang_NoClassDefFoundError;
struct vm_class *vm_java_lang_UnsatisfiedLinkError;
struct vm_class *vm_java_lang_ArrayIndexOutOfBoundsException;
struct vm_class *vm_java_lang_ArrayStoreException;
struct vm_class *vm_java_lang_RuntimeException;
struct vm_class *vm_java_lang_ExceptionInInitializerError;
struct vm_class *vm_java_lang_NoSuchFieldError;
struct vm_class *vm_java_lang_NoSuchMethodError;
struct vm_class *vm_java_lang_StackOverflowError;
struct vm_class *vm_java_lang_Thread;
struct vm_class *vm_java_lang_ThreadGroup;
struct vm_class *vm_java_lang_VMThread;
struct vm_class *vm_java_lang_IllegalMonitorStateException;
struct vm_class *vm_java_lang_System;
struct vm_class *vm_java_lang_reflect_Constructor;
struct vm_class *vm_java_lang_reflect_Field;
struct vm_class *vm_java_lang_reflect_Method;
struct vm_class *vm_array_of_java_lang_reflect_Constructor;
struct vm_class *vm_array_of_java_lang_reflect_Field;
struct vm_class *vm_array_of_java_lang_reflect_Method;
struct vm_class *vm_array_of_java_lang_Class;
struct vm_class *vm_java_lang_IllegalArgumentException;
struct vm_class *vm_java_lang_ClassLoader;
struct vm_class *vm_java_lang_Byte;
struct vm_class *vm_java_lang_Boolean;
struct vm_class *vm_java_lang_Character;
struct vm_class *vm_java_lang_Double;
struct vm_class *vm_java_lang_Float;
struct vm_class *vm_java_lang_Integer;
struct vm_class *vm_java_lang_Long;
struct vm_class *vm_java_lang_Short;
struct vm_class *vm_java_lang_VMString;
struct vm_class *vm_boolean_class;
struct vm_class *vm_char_class;
struct vm_class *vm_float_class;
struct vm_class *vm_double_class;
struct vm_class *vm_byte_class;
struct vm_class *vm_short_class;
struct vm_class *vm_int_class;
struct vm_class *vm_long_class;

static const struct preload_entry preload_entries[] = {
	{ "java/lang/Object",		&vm_java_lang_Object },
	{ "java/lang/Class",		&vm_java_lang_Class },
	{ "java/lang/Cloneable",	&vm_java_lang_Cloneable },
	{ "java/lang/String",		&vm_java_lang_String },
	{ "[Ljava/lang/String;",	&vm_array_of_java_lang_String },
	{ "java/lang/Throwable",	&vm_java_lang_Throwable },
	{ "java/util/Properties",	&vm_java_util_Properties },
	{ "java/lang/StackTraceElement", &vm_java_lang_StackTraceElement },
	{ "[Ljava/lang/StackTraceElement;", &vm_array_of_java_lang_StackTraceElement },
	{ "java/lang/VMThrowable",	&vm_java_lang_VMThrowable },
	{ "java/lang/ArithmeticException", &vm_java_lang_ArithmeticException },
	{ "java/lang/ArrayIndexOutOfBoundsException", &vm_java_lang_ArrayIndexOutOfBoundsException },
	{ "java/lang/ArrayStoreException", &vm_java_lang_ArrayStoreException },
	{ "java/lang/ClassCastException", &vm_java_lang_ClassCastException },
	{ "java/lang/ClassNotFoundException", &vm_java_lang_ClassNotFoundException },
	{ "java/lang/Error",		&vm_java_lang_Error },
	{ "java/lang/OutOfMemoryError",	 &vm_java_lang_OutOfMemoryError },
	{ "java/lang/ExceptionInInitializerError", &vm_java_lang_ExceptionInInitializerError },
	{ "java/lang/NegativeArraySizeException", &vm_java_lang_NegativeArraySizeException },
	{ "java/lang/NoClassDefFoundError", &vm_java_lang_NoClassDefFoundError },
	{ "java/lang/NullPointerException", &vm_java_lang_NullPointerException },
	{ "java/lang/RuntimeException",	&vm_java_lang_RuntimeException },
	{ "java/lang/UnsatisfiedLinkError", &vm_java_lang_UnsatisfiedLinkError },
	{ "java/lang/NoSuchFieldError", &vm_java_lang_NoSuchFieldError },
	{ "java/lang/NoSuchMethodError", &vm_java_lang_NoSuchMethodError },
	{ "java/lang/StackOverflowError", &vm_java_lang_StackOverflowError },
	{ "java/lang/Thread", &vm_java_lang_Thread },
	{ "java/lang/ThreadGroup", &vm_java_lang_ThreadGroup },
	{ "java/lang/VMThread",	&vm_java_lang_VMThread },
	{ "java/lang/IllegalMonitorStateException", &vm_java_lang_IllegalMonitorStateException },
	{ "java/lang/System",	&vm_java_lang_System },
	{ "java/lang/reflect/Field", &vm_java_lang_reflect_Field },
	{ "java/lang/reflect/Constructor", &vm_java_lang_reflect_Constructor },
	{ "java/lang/reflect/Method", &vm_java_lang_reflect_Method },
	{ "[java/lang/Class",		&vm_array_of_java_lang_Class },
	{ "[java/lang/reflect/Constructor", &vm_array_of_java_lang_reflect_Constructor },
	{ "[java/lang/reflect/Field", &vm_array_of_java_lang_reflect_Field },
	{ "[java/lang/reflect/Method", &vm_array_of_java_lang_reflect_Method },
	{ "java/lang/IllegalArgumentException", &vm_java_lang_IllegalArgumentException },
	{ "java/lang/Boolean", &vm_java_lang_Boolean },
	{ "java/lang/Byte", &vm_java_lang_Byte },
	{ "java/lang/Character", &vm_java_lang_Character },
	{ "java/lang/Short", &vm_java_lang_Short },
	{ "java/lang/Float", &vm_java_lang_Float },
	{ "java/lang/Integer", &vm_java_lang_Integer },
	{ "java/lang/Double", &vm_java_lang_Double },
	{ "java/lang/Long", &vm_java_lang_Long },
	{ "java/lang/ClassLoader", &vm_java_lang_ClassLoader},
	{ "java/lang/VMString", &vm_java_lang_VMString},
};

static const struct preload_entry primitive_preload_entries[] = {
	{"Z", &vm_boolean_class},
	{"C", &vm_char_class},
	{"F", &vm_float_class},
	{"D", &vm_double_class},
	{"B", &vm_byte_class},
	{"S", &vm_short_class},
	{"I", &vm_int_class},
	{"J", &vm_long_class},
};

struct field_preload_entry {
	struct vm_class **class;
	const char *name;
	const char *type;
	struct vm_field **field;
};

struct vm_field *vm_java_lang_Class_vmdata;
struct vm_field *vm_java_lang_String_offset;
struct vm_field *vm_java_lang_String_count;
struct vm_field *vm_java_lang_String_value;
struct vm_field *vm_java_lang_Throwable_detailMessage;
struct vm_field *vm_java_lang_VMThrowable_vmdata;
struct vm_field *vm_java_lang_Thread_daemon;
struct vm_field *vm_java_lang_Thread_group;
struct vm_field *vm_java_lang_Thread_name;
struct vm_field *vm_java_lang_Thread_priority;
struct vm_field *vm_java_lang_Thread_contextClassLoader;
struct vm_field *vm_java_lang_Thread_contextClassLoaderIsSystemClassLoader;
struct vm_field *vm_java_lang_Thread_vmThread;
struct vm_field *vm_java_lang_VMThread_thread;
struct vm_field *vm_java_lang_VMThread_vmdata;
struct vm_field *vm_java_lang_reflect_Constructor_clazz;
struct vm_field *vm_java_lang_reflect_Constructor_slot;
struct vm_field *vm_java_lang_reflect_Field_declaringClass;
struct vm_field *vm_java_lang_reflect_Field_name;
struct vm_field *vm_java_lang_reflect_Field_slot;
struct vm_field *vm_java_lang_reflect_Method_declaringClass;
struct vm_field *vm_java_lang_reflect_Method_name;
struct vm_field *vm_java_lang_reflect_Method_slot;

static const struct field_preload_entry field_preload_entries[] = {
	{ &vm_java_lang_Class, "vmdata", "Ljava/lang/Object;", &vm_java_lang_Class_vmdata },
	{ &vm_java_lang_String, "offset", "I",	&vm_java_lang_String_offset },
	{ &vm_java_lang_String, "count", "I",	&vm_java_lang_String_count },
	{ &vm_java_lang_String, "value", "[C",	&vm_java_lang_String_value },
	{ &vm_java_lang_Throwable, "detailMessage", "Ljava/lang/String;", &vm_java_lang_Throwable_detailMessage },
	{ &vm_java_lang_VMThrowable, "vmdata", "Ljava/lang/Object;", &vm_java_lang_VMThrowable_vmdata },
	{ &vm_java_lang_Thread, "daemon", "Z", &vm_java_lang_Thread_daemon },
	{ &vm_java_lang_Thread, "group", "Ljava/lang/ThreadGroup;", &vm_java_lang_Thread_group },
	{ &vm_java_lang_Thread, "name", "Ljava/lang/String;", &vm_java_lang_Thread_name },
	{ &vm_java_lang_Thread, "priority", "I", &vm_java_lang_Thread_priority },
	{ &vm_java_lang_Thread, "contextClassLoader", "Ljava/lang/ClassLoader;", &vm_java_lang_Thread_contextClassLoader },
	{ &vm_java_lang_Thread, "contextClassLoaderIsSystemClassLoader", "Z", &vm_java_lang_Thread_contextClassLoaderIsSystemClassLoader },
	{ &vm_java_lang_Thread, "vmThread", "Ljava/lang/VMThread;", &vm_java_lang_Thread_vmThread },
	{ &vm_java_lang_VMThread, "thread", "Ljava/lang/Thread;", &vm_java_lang_VMThread_thread },
	{ &vm_java_lang_VMThread, "vmdata", "Ljava/lang/Object;", &vm_java_lang_VMThread_vmdata },
	{ &vm_java_lang_reflect_Constructor, "clazz", "Ljava/lang/Class;", &vm_java_lang_reflect_Constructor_clazz },
	{ &vm_java_lang_reflect_Constructor, "slot", "I", &vm_java_lang_reflect_Constructor_slot },
	{ &vm_java_lang_reflect_Field, "declaringClass", "Ljava/lang/Class;", &vm_java_lang_reflect_Field_declaringClass },
	{ &vm_java_lang_reflect_Field, "slot", "I", &vm_java_lang_reflect_Field_slot },
	{ &vm_java_lang_reflect_Field, "name", "Ljava/lang/String;", &vm_java_lang_reflect_Field_name },
	{ &vm_java_lang_reflect_Method, "declaringClass", "Ljava/lang/Class;", &vm_java_lang_reflect_Method_declaringClass },
	{ &vm_java_lang_reflect_Method, "slot", "I", &vm_java_lang_reflect_Method_slot },
	{ &vm_java_lang_reflect_Method, "name", "Ljava/lang/String;", &vm_java_lang_reflect_Method_name },
};

struct method_preload_entry {
	struct vm_class **class;
	const char *name;
	const char *type;
	struct vm_method **method;
};

struct vm_method *vm_java_util_Properties_setProperty;
struct vm_method *vm_java_lang_Throwable_initCause;
struct vm_method *vm_java_lang_Throwable_getCause;
struct vm_method *vm_java_lang_Throwable_stackTraceString;
struct vm_method *vm_java_lang_Throwable_getStackTrace;
struct vm_method *vm_java_lang_Throwable_setStackTrace;
struct vm_method *vm_java_lang_StackTraceElement_init;
struct vm_method *vm_java_lang_Thread_init;
struct vm_method *vm_java_lang_Thread_isDaemon;
struct vm_method *vm_java_lang_Thread_getName;
struct vm_method *vm_java_lang_ThreadGroup_init;
struct vm_method *vm_java_lang_ThreadGroup_addThread;
struct vm_method *vm_java_lang_VMThread_init;
struct vm_method *vm_java_lang_VMThread_run;
struct vm_method *vm_java_lang_System_exit;
struct vm_method *vm_java_lang_Boolean_init;
struct vm_method *vm_java_lang_Byte_init;
struct vm_method *vm_java_lang_Character_init;
struct vm_method *vm_java_lang_Double_init;
struct vm_method *vm_java_lang_Float_init;
struct vm_method *vm_java_lang_Integer_init;
struct vm_method *vm_java_lang_Long_init;
struct vm_method *vm_java_lang_Short_init;
struct vm_method *vm_java_lang_ClassLoader_loadClass;
struct vm_method *vm_java_lang_VMString_intern;

static const struct method_preload_entry method_preload_entries[] = {
	{
		&vm_java_util_Properties,
		"setProperty",
		"(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;",
		&vm_java_util_Properties_setProperty,
	},
	{
		&vm_java_lang_Throwable,
		"initCause",
		"(Ljava/lang/Throwable;)Ljava/lang/Throwable;",
		&vm_java_lang_Throwable_initCause,
	},
	{
		&vm_java_lang_Throwable,
		"getCause",
		"()Ljava/lang/Throwable;",
		&vm_java_lang_Throwable_getCause,
	},
	{
		&vm_java_lang_Throwable,
		"getStackTrace",
		"()[Ljava/lang/StackTraceElement;",
		&vm_java_lang_Throwable_getStackTrace,
	},
	{
		&vm_java_lang_Throwable,
		"setStackTrace",
		"([Ljava/lang/StackTraceElement;)V",
		&vm_java_lang_Throwable_setStackTrace,
	},
	{
		&vm_java_lang_Throwable,
		"stackTraceString",
		"()Ljava/lang/String;",
		&vm_java_lang_Throwable_stackTraceString,
	},
	{
		&vm_java_lang_StackTraceElement,
		"<init>",
		"(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Z)V",
		&vm_java_lang_StackTraceElement_init,
	},
	{
		&vm_java_lang_Thread,
		"<init>",
		"(Ljava/lang/ThreadGroup;Ljava/lang/Runnable;Ljava/lang/String;)V",
		&vm_java_lang_Thread_init,
	},
	{
		&vm_java_lang_Thread,
		"isDaemon",
		"()Z",
		&vm_java_lang_Thread_isDaemon,
	},
	{
		&vm_java_lang_Thread,
		"getName",
		"()Ljava/lang/String;",
		&vm_java_lang_Thread_getName,
	},
	{
		&vm_java_lang_ThreadGroup,
		"<init>",
		"()V",
		&vm_java_lang_ThreadGroup_init,
	},
	{
		&vm_java_lang_ThreadGroup,
		"addThread",
		"(Ljava/lang/Thread;)V",
		&vm_java_lang_ThreadGroup_addThread,
	},
	{
		&vm_java_lang_VMThread,
		"<init>",
		"(Ljava/lang/Thread;)V",
		&vm_java_lang_VMThread_init,
	},
	{
		&vm_java_lang_VMThread,
		"run",
		"()V",
		&vm_java_lang_VMThread_run,
	},
	{
		&vm_java_lang_System,
		"exit",
		"(I)V",
		&vm_java_lang_System_exit,
	},
	{
		&vm_java_lang_Boolean,
		"<init>",
		"(Z)V",
		&vm_java_lang_Boolean_init,
	},
	{
		&vm_java_lang_Byte,
		"<init>",
		"(B)V",
		&vm_java_lang_Byte_init,
	},
	{
		&vm_java_lang_Character,
		"<init>",
		"(C)V",
		&vm_java_lang_Character_init,
	},
	{
		&vm_java_lang_Double,
		"<init>",
		"(D)V",
		&vm_java_lang_Double_init,
	},
	{
		&vm_java_lang_Long,
		"<init>",
		"(J)V",
		&vm_java_lang_Long_init,
	},
	{
		&vm_java_lang_Short,
		"<init>",
		"(S)V",
		&vm_java_lang_Short_init,
	},
	{
		&vm_java_lang_Float,
		"<init>",
		"(F)V",
		&vm_java_lang_Float_init,
	},
	{
		&vm_java_lang_Integer,
		"<init>",
		"(I)V",
		&vm_java_lang_Integer_init,
	},
	{
		&vm_java_lang_ClassLoader,
		"loadClass",
		"(Ljava/lang/String;)Ljava/lang/Class;",
		&vm_java_lang_ClassLoader_loadClass,
	},
	{
		&vm_java_lang_VMString,
		"intern",
		"(Ljava/lang/String;)Ljava/lang/String;",
		&vm_java_lang_VMString_intern,
	},
};

/*
 * Methods put in this table will be forcibly marked as native which
 * will allow VM to provide its own impementation for them.
 */
static struct vm_method **native_override_entries[] = {
	&vm_java_lang_VMString_intern,
};

int preload_vm_classes(void)
{
	unsigned int array_size;

	for (unsigned int i = 0; i < ARRAY_SIZE(preload_entries); ++i) {
		const struct preload_entry *pe = &preload_entries[i];

		struct vm_class *class = classloader_load(pe->name);
		if (!class) {
			printf("%s\n", pe->name);
			NOT_IMPLEMENTED;
			return 1;
		}

		*pe->class = class;
	}

	array_size = ARRAY_SIZE(primitive_preload_entries);
	for (unsigned int i = 0; i < array_size; ++i) {
		const struct preload_entry *pe = &primitive_preload_entries[i];

		struct vm_class *class = classloader_load_primitive(pe->name);
		if (!class) {
			NOT_IMPLEMENTED;
			return 1;
		}

		*pe->class = class;
	}

	for (unsigned int i = 0; i < ARRAY_SIZE(field_preload_entries); ++i) {
		const struct field_preload_entry *pe
			= &field_preload_entries[i];

		struct vm_field *field = vm_class_get_field(*pe->class,
			pe->name, pe->type);
		if (!field) {
			NOT_IMPLEMENTED;
			return 1;
		}

		*pe->field = field;
	}

	for (unsigned int i = 0; i < ARRAY_SIZE(method_preload_entries); ++i) {
		const struct method_preload_entry *me
			= &method_preload_entries[i];

		struct vm_method *method = vm_class_get_method(*me->class,
			me->name, me->type);
		if (!method) {
			warn("preload of %s.%s%s failed", (*me->class)->name,
			     me->name, me->type);
			NOT_IMPLEMENTED;
			return 1;
		}

		*me->method = method;
	}

	for (unsigned int i = 0; i < ARRAY_SIZE(native_override_entries); ++i) {
		struct cafebabe_method_info *m_info;
		struct compilation_unit *cu;
		struct vm_method *vmm;

		vmm = *native_override_entries[i];
		vmm->is_vm_native = true;

		cu = vmm->compilation_unit;

		cu->native_ptr = vm_lookup_native(vmm->class->name, vmm->name);
		if (!cu->native_ptr)
			error("no VM native for overriden method: %s.%s%s",
			      vmm->class->name, vmm->name, vmm->type);

		cu->is_compiled = true;

		if (add_cu_mapping((unsigned long)cu->native_ptr, cu)) {
			NOT_IMPLEMENTED;
			return -1;
		}

		m_info = (struct cafebabe_method_info *)vmm->method;
		m_info->access_flags |= CAFEBABE_METHOD_ACC_NATIVE;
	}

	return 0;
}
