
```

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

//-----------------------------------------------------------

#define CHAR_BYTES 1
#define SHORT_BYTES 2
#define INT_BYTES 4
#define FLOAT_BYTES 4
#define CHARS_PER_LINE 512

//-----------------------------------------------------------

/*

================
MD2 FORMAT NOTES
================

Bytes Per Type
--------------

char:  1
short: 2
int:   4
float: 4

Byte order: Little-Endian
-------------------------

With integers I1 and I2, least-significant and
most-significant bytes are ordered as follows.

[I1 LSB, ..., I1 MSB], [I2 LSB, ..., I2 MSB], ...

To analyze output file with od
------------------------------

Options:

-t c  // Print bytes as ASCII characters.
-t d4 // Print 4 byte groups as signed decimal numbers.
-t u1 // Print bytes as unsigned decimal numbers.
-t x1 // Print bytes as hexadecimal.
--width=1  // Print 4 bytes per line.  (Short option fails.)
-A d  // Use decimal radix for byte numbers.

Orientation
-----------

obj -> md2:

x -> y
y -> z
z -> x

*/

//-----------------------------------------------------------

void write_char(ofstream& md2_file, char c)
{
  md2_file.write(&c, 1);
}

//-----------------------------------------------------------

void write_unsigned_char(ofstream& md2_file, unsigned char c)
{
  md2_file.write((char*) &c, 1);
}

//-----------------------------------------------------------

void write_short(ofstream& md2_file, short s)
{
  md2_file.write((char*) &s, 2);
}

//-----------------------------------------------------------

void write_unsigned_short
(ofstream& md2_file, unsigned short s)
{
  md2_file.write((char*) &s, 2);
}

//-----------------------------------------------------------

void write_int(ofstream& md2_file, int i)
{
  md2_file.write((char*) &i, 4);
}

//-----------------------------------------------------------

void write_float(ofstream& md2_file, float f)
{
  md2_file.write((char*) &f, 4);
}

//-----------------------------------------------------------

void write_string
(ofstream& md2_file, const string& s, unsigned int length)
{
  for (unsigned int i = 0; i < length; ++i) {
    if (i < s.size())
      write_char(md2_file, s.at(i));
    else
      write_char(md2_file, '\0');
  }
}

//-----------------------------------------------------------

void gather_obj_data
(const string& obj_name, int& vertex_count, 
 int& texture_vertex_count, int& triangle_count)
{
  ifstream obj_file;
  char line_chars[CHARS_PER_LINE];
  istringstream line_stream;
  string first_string;

  // Open a representative obj file.
  obj_file.open(obj_name.c_str());

  // Initialize counts to 0.
  vertex_count = 0;
  texture_vertex_count = 0;
  triangle_count = 0;

  // Scan file, accumulating counts.
  // (Note: The items I want to count are all indicated by the
  // first string of a line.)
  // While there's something left in the file:
  while (obj_file.eof() == false) {
    // Read until next newline (or end of file).
    obj_file.getline(line_chars, CHARS_PER_LINE);
    line_stream.clear();
    line_stream.str(line_chars);
    line_stream >> first_string;
    if (!line_stream.fail()) {
      // A string was read.  Check for count items.
      if (first_string == "v")
        ++vertex_count;
      else if (first_string == "vt")
        ++texture_vertex_count;
      else if (first_string == "f")
        ++triangle_count;
    }
  }

  // Close obj file.
  obj_file.close();
}

//-----------------------------------------------------------

// Use the texture file name along with information from the
// first obj file to write the header to the md2 file.
void write_header
(ofstream& md2_file, const vector<string>& obj_names,
 int texture_width, int texture_height)
{
  int vertex_count;
  int texture_vertex_count;
  int triangle_count;
  int frame_count;
  int bytes_per_frame;
  int offset;

  // Write magic number "IDP2".
  write_char(md2_file, 'I');
  write_char(md2_file, 'D');
  write_char(md2_file, 'P');
  write_char(md2_file, '2');

  // Write version.  (Must be 8.)
  write_int(md2_file, 8);

  // Write texture width and height.
  write_int(md2_file, texture_width);
  write_int(md2_file, texture_height);

  // Gather data from obj files.
  gather_obj_data
    (obj_names.front(), vertex_count, texture_vertex_count,
     triangle_count);
  frame_count = obj_names.size();

  // Write number of bytes per frame.
  bytes_per_frame =
    3 * FLOAT_BYTES + // scale
    3 * FLOAT_BYTES + // translate
    16 * CHAR_BYTES + // name
    vertex_count * (4 * CHAR_BYTES); // vertices
  write_int(md2_file, bytes_per_frame);

  // Write number of skins.  (Like uniforms.  Not used here.)
  write_int(md2_file, 1);

  // Write number of vertices.
  write_int(md2_file, vertex_count);

  // Write number of texture vertices.
  write_int(md2_file, texture_vertex_count);

  // Write number of triangles.
  write_int(md2_file, triangle_count);

  // Write number of OpenGL commands.  (Not used here.)
  write_int(md2_file, 0);

  // Write number of frames.
  write_int(md2_file, frame_count);

  // Compute size of entire header to use as first explicit
  // offset value.  (17 ints will be written altogether.)
  offset = 17 * INT_BYTES;

  // Write offset for skin data and compute next offset.
  // (Skin data is a 64-byte string for texture filename, and
  // I use only one such file.)
  write_int(md2_file, offset);
  offset += 1 * (64 * CHAR_BYTES);

  // Write offset for texture vertex data and compute next
  // offset.
  // (A texture vertex is one short for each dimension.)
  write_int(md2_file, offset);
  offset += texture_vertex_count * (2 * SHORT_BYTES);

  // Write offset for triangle data and compute next offset.
  // (A triangle is 3 shorts of vertex indices + 3 shorts of
  // texture vertex indices.)
  write_int(md2_file, offset);
  offset += triangle_count * (6 * SHORT_BYTES);

  // Write offset for frame data and compute next offset.
  // (Frame data size is number of frames times bytes per
  // frame.)
  write_int(md2_file, offset);
  offset += frame_count * bytes_per_frame;

  // Write offset for OpenGL commands.  (Not used.)
  // (Since there is no data of this type, the next offset
  // will be the same as this one.)
  write_int(md2_file, offset);

  // Write offset for end of file.
  // (I assume this means the offset for the first byte after
  // the last byte of data.)
  write_int(md2_file, offset);
}

//-----------------------------------------------------------

// Convert texture coordinates from [0,1] to 
// [0, width|height - 1].
// Note: Orientation of texture vertical dimension (v or t) is
// opposite of that used in obj.
void try_to_copy_texture_vertex
(ofstream& md2_file, const char* line_chars,
 int texture_width, int texture_height)
{
  istringstream line_stream;
  string one_string;
  double old_u;
  double old_v;
  short new_u;
  short new_v;

  // Check if this line contains a texture vertex.
  line_stream.clear();
  line_stream.str(line_chars);
  line_stream >> one_string;
  if (line_stream.fail()) {
    return;
  }
  else {
    if (one_string != "vt")
      return;
  }

  // Get u and v in [0,1] from obj line.
  line_stream >> old_u;
  line_stream >> old_v;

  // Convert to u and v in [0, width|height - 1], flipping
  // vertical value.
  // (Truncation in assignment to new_u|v is intentional.)
  new_u = old_u * texture_width;
  new_v = (1.0 - old_v) * texture_height;
  if (new_u >= texture_width)
    new_u = texture_width;
  if (new_v >= texture_height)
    new_v = texture_height;

  // Write result to md2 file.
  write_short(md2_file, new_u);
  write_short(md2_file, new_v);
}

//-----------------------------------------------------------

// Note: Vertex order in face entries is v/vt/vn.
// Note: obj indices start at 1.  
// Question: What index do md2 vertices start at?
// I'll assume 0 until proven otherwise.
void try_to_copy_triangle
(ofstream& md2_file, const char* line_chars)
{
  istringstream line_stream;
  string one_string;
  unsigned short vertices[3];
  unsigned short texture_vertices[3];

  // Check if this line contains a triangle (face).
  line_stream.clear();
  line_stream.str(line_chars);
  line_stream >> one_string;
  if (line_stream.fail()) {
    return;
  }
  else {
    if (one_string != "f")
      return;
  }

  // Repeatedly get vertex and texture vertex indices from
  // obj.
  // (Note: I may be assuming that texture vertices exist in
  // this parsing.)
  for (int i = 0; i < 3; ++i) {
    line_stream >> vertices[i]; // Get v.
    line_stream.get(); // Skip '/'.
    line_stream >> texture_vertices[i]; // Get vt.
    line_stream.get(); // Skip '/'.
    line_stream >> one_string; // Skip vn.
  }

  // Write triangle to md2.
  // Note: I'll assume that md2 uses indices from 0, but I
  // have no real information on this yet.
  // Note: I think they specify triangles in the opposite
  // order as obj.  Reverse during write.
  for (int i = 2; i >= 0; --i)
    write_unsigned_short(md2_file, vertices[i] - 1);
  for (int i = 2; i >= 0; --i)
    write_unsigned_short(md2_file, texture_vertices[i] - 1);
}

//-----------------------------------------------------------

void write_post_header_data
(ofstream& md2_file, const vector<string>& obj_names, 
 const string& texture_name, int texture_width, 
 int texture_height)
{
  ifstream obj_file;
  char line_chars[CHARS_PER_LINE];

  // Write skin (texture) name as 64 chars.
  write_string(md2_file, texture_name, 64);

  // Open representative obj file.
  obj_file.open(obj_names.front().c_str());
  
  // Gather and write texture vertices.
  while (obj_file.eof() == false) {
    obj_file.getline(line_chars, CHARS_PER_LINE);
    try_to_copy_texture_vertex
      (md2_file, line_chars, texture_width, texture_height);
  }

  // Rewind obj file.
  // (Note: Apparently clear must be called before seekg for
  // this to work.)
  obj_file.clear();
  obj_file.seekg(0);

  // Gather and write triangles.
  while (obj_file.eof() == false) {
    obj_file.getline(line_chars, CHARS_PER_LINE);
    try_to_copy_triangle(md2_file, line_chars);
  }

  // Close obj file.
  obj_file.close();
}

//-----------------------------------------------------------

// Note: Define scale and translate so as to go from
// compressed (unsigned char) to uncompressed (float).
void compute_scale_and_translate
(const string& obj_name, float* scale, float* translate)
{
  ifstream obj_file;
  char line_chars[CHARS_PER_LINE];
  bool min_and_max_initialized;
  istringstream line_stream;
  string one_string;
  float vertex_value;
  float min[3];
  float max[3];

  // Find min and max of vertices in each dimension.  (Ugly?)
  obj_file.open(obj_name.c_str());
  min_and_max_initialized = false;
  while (obj_file.eof() == false) {
    obj_file.getline(line_chars, CHARS_PER_LINE);
    line_stream.clear();
    line_stream.str(line_chars);
    line_stream >> one_string;
    if (!line_stream.fail() && one_string == "v") {
      for (int i = 0; i <  3; ++i) {
        line_stream >> vertex_value;
        if (vertex_value < min[i] || !min_and_max_initialized)
          min[i] = vertex_value;
        if (vertex_value > max[i] || !min_and_max_initialized)
          max[i] = vertex_value;
      }
      min_and_max_initialized = true;
    }
  }
  obj_file.close();

  // Compute translate.
  translate[0] = min[0];
  translate[1] = min[1];
  translate[2] = min[2];

  // Compute scale.
  // (Unsigned char of compressed version is in [0,255].)
  scale[0] = (max[0] - min[0]) / 255.0;
  scale[1] = (max[1] - min[1]) / 255.0;
  scale[2] = (max[2] - min[2]) / 255.0;
}

//-----------------------------------------------------------

// Write this obj's data to the md2 file.
void write_frame(ofstream& md2_file, const string& obj_name)
{
  float scale[3];
  float translate[3];
  string obj_name_base;
  ifstream obj_file;
  char line_chars[CHARS_PER_LINE];
  istringstream line_stream;
  string one_string;
  float old_vertex_value;
  unsigned char new_vertex_values[3];

  // Compute scale and translate.
  compute_scale_and_translate(obj_name, scale, translate);

  // Write scale (with change from obj to md2 space).
  write_float(md2_file, scale[2]);
  write_float(md2_file, scale[0]);
  write_float(md2_file, scale[1]);

  // Write translate (with change from obj to md2 space).
  write_float(md2_file, translate[2]);
  write_float(md2_file, translate[0]);
  write_float(md2_file, translate[1]);

  // Write frame name as 16 chars.  (Use obj name without
  // extension.)
  obj_name_base = obj_name.substr(0, obj_name.size() - 4);
  write_string(md2_file, obj_name_base, 16);

  // Write vertices.
  // Note: I haven't checked for boundary/truncation problems.
  obj_file.open(obj_name.c_str());
  while (obj_file.eof() == false) {
    obj_file.getline(line_chars, CHARS_PER_LINE);
    line_stream.clear();
    line_stream.str(line_chars);
    line_stream >> one_string;
    if (!line_stream.fail() && one_string == "v") {
      // Compute compressed vertex.
      for (int i = 0; i <  3; ++i) {
        line_stream >> old_vertex_value;
        new_vertex_values[i] =
          (old_vertex_value - translate[i]) / scale[i];
      }
      // Write compressed vertex with change from obj to md2
      // space.
      write_unsigned_char(md2_file, new_vertex_values[2]);
      write_unsigned_char(md2_file, new_vertex_values[0]);
      write_unsigned_char(md2_file, new_vertex_values[1]);
      // Write indexed normal.
      // Note: Assuming index from 0, but I don't really know.
      // (This actually may cause problems, since I think it
      // requires one vertex normal per vertex.)
      /* HACK */
      // Fake for now.
      write_unsigned_char(md2_file, 0);
      /* HACK */
    }
  }
  obj_file.close();
}

//-----------------------------------------------------------

int main(int argc, char** argv)
{
  int i;
  vector<string> obj_names;
  string texture_name;
  istringstream iss;
  int texture_width;
  int texture_height;
  string md2_name;
  ofstream md2_file;
  vector<string>::const_iterator obj_name_iter;

  // If no arguments, show usage and exit.
  if (argc == 1) {
    cout << "usage: obj_to_md2 "
         << "<obj_1> ... <obj_n> "
         << "<texture_name> <texture_width> <texture_height> "
         << "<md2_name>"
         << endl;
    exit(0);
  }

  // Parse arguments.
  for (i = 1; i < argc - 4; ++i)
    obj_names.push_back(argv[i]);
  texture_name = argv[argc - 4];
  iss.clear();
  iss.str(argv[argc - 3]);
  iss >> texture_width;
  iss.clear();
  iss.str(argv[argc - 2]);
  iss >> texture_height;
  md2_name = argv[argc - 1];

  // Open output file.
  md2_file.open(md2_name.c_str(), ofstream::binary);

  // Write header.
  write_header
    (md2_file, obj_names, texture_width, texture_height);

  // Write non-frame post-header data.
  write_post_header_data
    (md2_file, obj_names, texture_name, texture_width,
     texture_height);

  // For each obj, write frame to md2.
  for (obj_name_iter = obj_names.begin();
       obj_name_iter != obj_names.end();
       ++obj_name_iter)
    write_frame(md2_file, *obj_name_iter);

  // Close output file.
  md2_file.close();
}

//-----------------------------------------------------------

```