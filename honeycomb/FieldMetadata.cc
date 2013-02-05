#include "FieldMetadata.h"
#include "Util.h"
#include "JNICache.h"
#include "Macros.h"
#include "Logging.h"

FieldMetadata::FieldMetadata(JNIEnv* env, JNICache* cache) : env(env), cache(cache)
{
}

jobject FieldMetadata::get_field_metadata(Field *field, TABLE *table_arg, ulonglong auto_increment_value)
{
  jclass metadata_class = cache->column_metadata().clazz;

  jmethodID metadata_constructor = cache->column_metadata().init;
  jmethodID set_max_length_method = cache->column_metadata().set_max_length;
  jmethodID set_precision_method = cache->column_metadata().set_precision;
  jmethodID set_scale_method = cache->column_metadata().set_scale;
  jmethodID set_nullable_method = cache->column_metadata().set_nullable;
  jmethodID set_primary_key_method = cache->column_metadata().set_primary_key;
  jmethodID set_type_method = cache->column_metadata().set_type;
  jmethodID set_autoincrement_method = cache->column_metadata().set_autoincrement;
  jmethodID set_autoincrement_value_method = cache->column_metadata().set_autoincrement_value;

  jobject metadata_object = this->env->NewObject(metadata_class, metadata_constructor);
  EXCEPTION_CHECK_ABORT("FieldMetadata::get_field_metadata: OutOfMemoryException while calling NewObject");
  JNICache::ColumnType column_type = cache->column_type();

  switch (field->real_type())
  {
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_YEAR:
      if (is_unsigned_field(field))
      {
        env->CallVoidMethod(metadata_object, set_type_method,
            env->GetStaticObjectField(column_type.clazz, column_type.ULONG));
        EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setType");
      }
      else
      {
        env->CallVoidMethod(metadata_object, set_type_method,
            env->GetStaticObjectField(column_type.clazz, column_type.LONG));
        EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setType");
      }
      this->env->CallVoidMethod(metadata_object, set_max_length_method, (jint)8);
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setMaxLength");
      break;
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      env->CallVoidMethod(metadata_object, set_type_method,
          env->GetStaticObjectField(column_type.clazz, column_type.DOUBLE));
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setType");
      this->env->CallVoidMethod(metadata_object, set_max_length_method, (jint)8);
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setMaxLength");
      break;
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
      {
        uint precision = ((Field_new_decimal*) field)->precision;
        uint scale = ((Field_new_decimal*) field)->dec;
        env->CallVoidMethod(metadata_object, set_type_method,
            env->GetStaticObjectField(column_type.clazz, column_type.DECIMAL));
        EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setType");
        this->env->CallVoidMethod(metadata_object, set_precision_method, (jint)precision);
        EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setPrecision");
        this->env->CallVoidMethod(metadata_object, set_scale_method, (jint)scale);
        EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setScale");
        this->env->CallVoidMethod(metadata_object, set_max_length_method, (jint)field->field_length);
        EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setMaxLength");
      }
      break;
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_NEWDATE:
      env->CallVoidMethod(metadata_object, set_type_method,
          env->GetStaticObjectField(column_type.clazz, column_type.DATE));
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setType");
      this->env->CallVoidMethod(metadata_object, set_max_length_method, (jint)field->field_length);
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setMaxLength");
      break;
    case MYSQL_TYPE_TIME:
      env->CallVoidMethod(metadata_object, set_type_method,
          env->GetStaticObjectField(column_type.clazz, column_type.TIME));
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setType");
      this->env->CallVoidMethod(metadata_object, set_max_length_method, (jint)field->field_length);
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setMaxLength");
      break;
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
      env->CallVoidMethod(metadata_object, set_type_method,
          env->GetStaticObjectField(column_type.clazz, column_type.DATETIME));
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setType");
      this->env->CallVoidMethod(metadata_object, set_max_length_method, (jint)field->field_length);
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setMaxLength");
      break;
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VARCHAR:
      {
        long long max_char_length = (long long) field->field_length;

        this->env->CallVoidMethod(metadata_object, set_max_length_method, (jint)max_char_length);
        EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setMaxLength");

        if (field->binary())
        {
          env->CallVoidMethod(metadata_object, set_type_method,
              env->GetStaticObjectField(column_type.clazz, column_type.BINARY));
          EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setType");
        }
        else
        {
          env->CallVoidMethod(metadata_object, set_type_method,
              env->GetStaticObjectField(column_type.clazz, column_type.STRING));
          EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setType");
        }
      }
      break;
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
      env->CallVoidMethod(metadata_object, set_type_method,
          env->GetStaticObjectField(column_type.clazz, column_type.BINARY));
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setType");
      break;
    case MYSQL_TYPE_ENUM:
      env->CallVoidMethod(metadata_object, set_type_method,
          env->GetStaticObjectField(column_type.clazz, column_type.ULONG));
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setType");
      break;
    case MYSQL_TYPE_NULL:
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_VAR_STRING:
    default:
      break;
  }

  if (field->real_maybe_null())
  {
    this->env->CallVoidMethod(metadata_object, set_nullable_method, JNI_TRUE);
    EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setNullable");
  }

  if(table_arg->found_next_number_field != NULL && field == table_arg->found_next_number_field)
  {
    this->env->CallVoidMethod(metadata_object, set_autoincrement_method, JNI_TRUE);
    EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setAutoincrement");
    this->env->CallVoidMethod(metadata_object, set_autoincrement_value_method,
        auto_increment_value == 0 ? 1 : auto_increment_value);
    EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setAutoincrementValue");
  }

  // 64 is obviously some key flag indicating no primary key, but I have no
  // idea where it's defined. Will fix later. - ABC
  if (table_arg->s->primary_key != 64)
  {
    if (strcmp(table_arg->s->key_info[table_arg->s->primary_key].key_part->field->field_name,
          field->field_name) == 0)
    {
      this->env->CallVoidMethod(metadata_object, set_primary_key_method, JNI_TRUE);
      EXCEPTION_CHECK("FieldMetadata::get_field_metadata", "calling setPrimaryKey");
    }
  }
  return metadata_object;
}
