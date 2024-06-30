#include <stdio.h>

#define FS_IMPLEMENTATION
#include <core/fs.h>

#define STR_IMPLEMENTATION
#include <core/str.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <thirdparty/stb_image_write.h>

// Decoding

#define QOI_IMPLEMENTATION
#include <thirdparty/qoi.h>

#define STB_IMAGE_IMPLEMENTATION
#include <thirdparty/stb_image.h>

#define PNM_IMPLEMENTATION
#include <core/pnm.h>

#include <core/types.h>

char *next(int *argc, char ***argv) {
  if((*argc) == 0) {
    return NULL;
  }

  char *next = (*argv)[0];

  *argc = *argc - 1;
  *argv = *argv + 1;

  return next;
}

typedef enum {
  FORMAT_TYPE_NONE = 0,
  FORMAT_TYPE_PNG,
  FORMAT_TYPE_JPG,
  FORMAT_TYPE_TGA,
  FORMAT_TYPE_BMP,
  FORMAT_TYPE_HDR,
  FORMAT_TYPE_PNM,
  FORMAT_TYPE_QOI,
}Format_Type;

const char *format_type_names[] = {
  [FORMAT_TYPE_NONE] = "None",

  //https://en.wikipedia.org/wiki/PNG
  [FORMAT_TYPE_PNG]  = "Portable Network Graphics (png)",

  //https://en.wikipedia.org/wiki/JPEG
  [FORMAT_TYPE_JPG]  = "Joint Photographic Experts Group (jpg/jpeg)",
  
  //https://en.wikipedia.org/wiki/Truevision_TGA
  [FORMAT_TYPE_TGA]  = "Truevision TGA (tga)",

  //https://en.wikipedia.org/wiki/BMP_file_format
  [FORMAT_TYPE_BMP]  = "BMP file format (bmp)",

  //https://docs.fileformat.com/image/hdr/
  [FORMAT_TYPE_HDR]  = "High Dynamic Range (hdr)",

  //https://en.wikipedia.org/wiki/Netpbm#File_formats
  [FORMAT_TYPE_PNM] = "Portable Anymap (pnm)",

  //https://de.wikipedia.org/wiki/Quite_OK_Image_Format
  [FORMAT_TYPE_QOI]  = "Quite OK Image Format (qoi)",
};

#define format_type_name(t) format_type_names[(t)]

typedef struct{
  const char *name;
  Format_Type type;
}Assoc;

#define assoc_from(t, n) (Assoc) { (t), (n) }

// This program relies on the grouping of successive
// formats. E.g.
//   'png', 'jpg', 'jpeg' => valid
//   'jpg', 'png', 'jpeg' => invalid

Assoc assocs[] = {
  { "png", FORMAT_TYPE_PNG },

  { "jpg", FORMAT_TYPE_JPG },
  { "jpeg", FORMAT_TYPE_JPG },
  { "jpe", FORMAT_TYPE_JPG },
  { "jif", FORMAT_TYPE_JPG },
  { "jfif", FORMAT_TYPE_JPG },
  { "jfi", FORMAT_TYPE_JPG},

  { "tga", FORMAT_TYPE_TGA },
  { "icb", FORMAT_TYPE_TGA },
  { "vda", FORMAT_TYPE_TGA },
  { "vst", FORMAT_TYPE_TGA },

  { "bmp", FORMAT_TYPE_BMP },
  { "dip", FORMAT_TYPE_BMP },

  { "hdr", FORMAT_TYPE_HDR },

  { "pnm", FORMAT_TYPE_PNM },
  { "ppm", FORMAT_TYPE_PNM },
  { "pgm", FORMAT_TYPE_PNM },
  { "pam", FORMAT_TYPE_PNM },
    
  { "qoi", FORMAT_TYPE_QOI },

};

Format_Type format_type_for_name(str name) {
  
  for(size_t i=0;i<sizeof(assocs)/sizeof(assocs[0]);i++) {
    Assoc *assoc = &assocs[i];
    
    if(str_eqc(name, assoc->name)) {
      return assoc->type;
    }
  }

  return FORMAT_TYPE_NONE;
}

Format_Type format_type_for_filename(str filename) {
  if(!filename.len) return FORMAT_TYPE_NONE;

  str last = filename;
  while(filename.len) {
    str part;
    str_chop_by(&filename, ".", &part);
    last = part;
  }

  return format_type_for_name(last);
}

// Memory is intentionally not freed
int transform(Assoc input, Assoc output) {
  assert(input.type != FORMAT_TYPE_NONE &&
	 output.type != FORMAT_TYPE_NONE);

  int width = 0, height = 0, channels = 0;
  unsigned char *data = NULL;
  switch(input.type) {
  case FORMAT_TYPE_NONE: 
    return 1; // unreachable
  case FORMAT_TYPE_PNG:
  case FORMAT_TYPE_JPG:
  case FORMAT_TYPE_TGA:
  case FORMAT_TYPE_BMP:
  case FORMAT_TYPE_HDR: {
    data = stbi_load(input.name, &width, &height, &channels, 0);
  } break;
  case FORMAT_TYPE_PNM: {
    data = pnm_load(input.name, &width, &height, &channels, 0);
  } break;
  case FORMAT_TYPE_QOI: {
    qoi_desc desc;
    data = qoi_read(input.name, &desc, 0);
    width = desc.width;
    height = desc.height;
    channels = desc.channels;
  } break;
  }

  if(!data) {
    fprintf(stderr, "ERROR: Can not decode '%s' expected format: '%s'\n",
	    input.name, format_type_name(input.type));
    return 1;
  }

  bool success = false;
  switch(output.type) {
  case FORMAT_TYPE_NONE:
    return 1; //unreachable
  case FORMAT_TYPE_PNG: {
    success = stbi_write_png(output.name,
			     width, height, channels,
			     data,
			     channels * width) != 0;
  } break;
  case FORMAT_TYPE_JPG: {
    success = stbi_write_jpg(output.name,
			     width, height, channels,
			     data,
			     width * channels) != 0;
  } break;
  case FORMAT_TYPE_TGA: {
    success = stbi_write_tga(output.name,
			     width, height, channels,
			     data) != 0;
  } break;
  case FORMAT_TYPE_BMP: {
    success = stbi_write_bmp(output.name,
			     width, height, channels,
			     data) != 0;
  } break;
  case FORMAT_TYPE_HDR: {

    float *hdr_pixels = malloc(width * height * sizeof(float) * channels);
    assert(hdr_pixels);
    for(size_t y=0;y<height;y++) {
      for(size_t x=0;x<width;x++) {

	switch(channels) {
	case 1:
	case 2: {
	  unsigned char *pixels = &data[(y * width + x) * channels];
	  hdr_pixels[(y * width + x) * channels + 0] = (float) pixels[0] / 255.f;
	} break;
	case 3:
	case 4: { 
	  unsigned char *pixels = &data[(y * width + x) * channels];
	  hdr_pixels[(y * width + x) * channels + 0] = (float) pixels[0] / 255.f;
	  hdr_pixels[(y * width + x) * channels + 1] = (float) pixels[1] / 255.f;
	  hdr_pixels[(y * width + x) * channels + 2] = (float) pixels[2] / 255.f;
	} break;
	}	
	
      }
    }
    
    success = stbi_write_hdr(output.name,
			     width, height, channels,
			     hdr_pixels) != 0;
  } break;
  case FORMAT_TYPE_PNM: {

    success = pnm_write(output.name,
			width, height, channels,
			data) != 0;
    
  } break;
  case FORMAT_TYPE_QOI: {
    success = qoi_write(output.name, data, &(qoi_desc) {
	width,
	height,
	(unsigned char) channels,
        QOI_SRGB
      }) != 0;
  } break;
  }

  if(!success) {
    fprintf(stderr, "ERROR: Could not encode '%s' to format: '%s'\n",
	    output.name, format_type_name(output.type));
    return 1;
  }

  size_t input_type_len = strlen(format_type_name(input.type));
  size_t input_len = strlen(input.name);

  printf("INFO: Finished converting. size: (%d, %d), channels: %d\n",
	 width, height, channels);
  
  size_t input_dt;    
  if(input_type_len > input_len) {
    input_dt = input_type_len - input_len;

    printf("       '%s'", input.name);
    for(size_t i=0;i<=input_dt;i++) printf(" ");
    printf("-> '%s'\n"
	   "       '%s' -> '%s'\n",
	   output.name,
	   format_type_name(input.type), format_type_name(output.type));
    
  } else {
    input_dt = input_len - input_type_len;

    printf("       '%s' -> '%s'\n", input.name, output.name);
    printf("       '%s'", format_type_name(input.type));
    for(size_t i=0;i<=input_dt;i++) printf(" ");
    printf("-> '%s' \n", format_type_name(output.type));
  }

  
  
  
  return 0;
}

int invalid_format_flag(const char *flag) {
  fprintf(stderr, "ERROR: '%s' does not correspond to any format\n\n", flag);
  fprintf(stderr, "Supported formats and flags are:");

  Format_Type last_type = FORMAT_TYPE_NONE;
  for(size_t i=0;i<sizeof(assocs)/sizeof(assocs[0]);i++) {
    Assoc *assoc = &assocs[i];
    assert(assoc->type != FORMAT_TYPE_NONE);
    
    if(last_type == FORMAT_TYPE_NONE || assoc->type != last_type) {
      
      fprintf(stderr, "\n\n");
      last_type = assoc->type;
      fprintf(stderr, "    %s: '-%s'", format_type_name(last_type), assoc->name);
    } else {
      fprintf(stderr, ", '-%s'", assoc->name);
    }
     
  }
  
  return 1;
}

int not_enough_arguments(const char *program) {
  fprintf(stderr, "ERROR  : Please provide enough arguments\n");
  fprintf(stderr, "USAGE  : %s [input-format] <input> [output-format] <output>\n", program);
  fprintf(stderr, "EXAMPLE: %s file1.png -jpg file2\n", program);
  return 1;
}

int main(int argc, char **argv) {
  
  char *input = NULL;
  Format_Type input_format = FORMAT_TYPE_NONE;
  char *output = NULL;
  Format_Type output_format = FORMAT_TYPE_NONE;

  char *program = next(&argc, &argv);

  char *in = next(&argc, &argv);
  if(!in) {
    return not_enough_arguments(program);
  }
  if(in[0] == '-') {
    input_format = format_type_for_name(str_from((u8 *) in + 1, strlen(in) - 1));
    if(input_format == FORMAT_TYPE_NONE) {
      return invalid_format_flag(in);
    }    
    in = next(&argc, &argv);
  }
  if(!in) {
    return not_enough_arguments(program);
  }
  input = in;

  in = next(&argc, &argv);
  if(!in) {
    return not_enough_arguments(program);
  }
  if(in[0] == '-') {
    output_format = format_type_for_name(str_from((u8 *) in + 1, strlen(in) - 1));
    if(output_format == FORMAT_TYPE_NONE) {
      return invalid_format_flag(in);
    }    
    in = next(&argc, &argv);
  }
  if(!in) {
    return not_enough_arguments(program);
  }
  output = in;

  /////////////////////////////////////////////////////

  int is_file;
  if(!fs_existsc(input, &is_file)) {
    fprintf(stderr, "ERROR: '%s' does not exist\n", input);
    return 1;
  }
  if(!is_file) {
    fprintf(stderr, "ERROR: '%s' is not a file\n", input);
    return 1;    
  }
  if(input_format == FORMAT_TYPE_NONE) {
    input_format = format_type_for_filename(str_fromc(input));
  }
  if(input_format == FORMAT_TYPE_NONE) {
    fprintf(stderr, "ERROR: Can not find a suitable format for input: '%s'\n", input);
    return 1;
  }

  if(output_format == FORMAT_TYPE_NONE) {
    output_format = format_type_for_filename(str_fromc(output));
  }
  if(output_format == FORMAT_TYPE_NONE) {
    fprintf(stderr, "ERROR: Can not find a suitable format for output: '%s'\n", input);
    return 1;
  }

  return transform(assoc_from(input, input_format), assoc_from(output, output_format));
}
