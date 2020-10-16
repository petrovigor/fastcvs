#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>

using namespace std;

const int prop_count = 7; //structure property count
const int max_line_len = 1024;

enum prop_type {
  prop_type_int,
  prop_type_float,
  prop_type_string,
  prop_type_bool
};

struct link_prop {
  char file_prop_name[64];
  int str_field_offset;
  int prop_file_pos;
  prop_type type;
} link[prop_count];

#pragma pack(push, 1)
struct person_data {
  int age;
  bool sex;
  float height;
  float weight;
  float temperature;
  float pressure_upper;
  float pressure_lower;
} temp_person;

#pragma pop()

void init_link_data() {
  int n_prop = 0;

#define do_link_prop(struct_name_prop, file_prop_name_, type_) \
  strcpy(link[n_prop].file_prop_name, file_prop_name_); \
  link[n_prop].str_field_offset = offsetof(person_data, struct_name_prop); \
  link[n_prop++].type = type_;
  
  do_link_prop(age, "age", prop_type_int);
  do_link_prop(sex, "sex", prop_type_bool);
  do_link_prop(height, "height", prop_type_float);
  do_link_prop(weight, "weight", prop_type_float);
  do_link_prop(temperature, "temperature", prop_type_float);
  do_link_prop(pressure_upper, "pressure_upper", prop_type_float);
  do_link_prop(pressure_lower, "pressure_lower", prop_type_float);

#undef do_link_prop
}

bool is_word_empty(char* str, int len) {
  for(int i = 0; i < len; i++) {
    if(str[i] && str[i] != ' ') {
      return false;
    }
  }
  return true;
}

typedef void (*split_word_cb)(char*, int, int, void*);
void split_line(char* line, int len, split_word_cb cb, void* arg = nullptr) {
  int start = 0;
  int cur = start;
  int word_n = 0;

  while(line[cur]) {
    if(line[cur] == ' ') {
      if(!is_word_empty(line + start, cur - start)) {
        cb(line + start, cur - start, word_n++, arg);
      }
      start = cur + 1;
    }
    cur++;
  }
  if(!is_word_empty(line + start, cur - start)) {
    cb(line + start, cur - start, word_n, arg);
  }
}

void parse_prop(char* p, int len, int prop_n, void*) {
  if(p[len - 1] == ',')
    len -= 1;

  if(prop_n > prop_count) {
    cerr << "Warning! Too many properties in the header (first line) of the file! Maximum = " << prop_count << endl;
  }

  char prop[64];
  strncpy(prop, p, len);
  prop[len] = 0;

  bool prop_found = false;
  for(int i = 0; i < prop_count; i++) {
    if(!strcmp(link[i].file_prop_name, prop)) {
      link[i].prop_file_pos = prop_n;
      cout << "Link " << prop << " with index " << link[i].prop_file_pos << " and offset " << link[i].str_field_offset << endl;
      prop_found = true;
      break;
    }
  }

  if(!prop_found) {
    cerr << "Warning! Unknown property in the header (first line) of the file: \"" << prop << "\"" << endl;
  }
}

void print_person(const person_data& data) {
  cout << "Person: " << endl <<
          "age: " << data.age << endl <<
          "sex: " << data.sex << endl <<
          "height: " << data.height << endl <<
          "weight: " << data.weight << endl <<
          "temperature: " << data.temperature << endl <<
          "pressure_upper: " << data.pressure_upper << endl <<
          "pressure_lower: " << data.pressure_lower << endl;
}

void parse_person(char* p, int len, int prop_n, void* arg) {
  if(p[len - 1] == ',')
    len -= 1;

  if(prop_n > prop_count) {
    cerr << "Warning! Too many properties on the line! Maximum = " << prop_count << endl;
  }

  char prop[64];
  strncpy(prop, p, len);
  prop[len] = 0;

  for(int i = 0; i < prop_count; i++) {
    if(link[i].prop_file_pos == prop_n) {
      unsigned char* offset = ((unsigned char*)arg) + link[i].str_field_offset;

      switch(link[i].type) {
        case prop_type_int:
          *((int*)offset) = atoi(prop);
          break;
        case prop_type_bool:
          *((bool*)offset) = (atoi(prop) != 0);
          break;
        case prop_type_string:
          strncpy(((char*)offset), prop, sizeof(prop));
          break;
        case prop_type_float:
          *((float*)offset) = (float)atof(prop);
          break;
      }
    }
  }
}

int main() {
  init_link_data();
  
  const char first_line[max_line_len] = "  sex,     weight,  temperature,  pressure_lower, pressure_upper, age, height     ";
  const char line[max_line_len] = "\"male\",         60,      36.6,     190,   150, 25, 180";

  split_line((char*)first_line, strlen(first_line), parse_prop);
  split_line((char*)line, strlen(line), parse_person, &temp_person);

  print_person(temp_person);

  return 0;
}
