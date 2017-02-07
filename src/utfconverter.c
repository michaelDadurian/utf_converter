#include "../include/utfconverter.h"
#ifdef __sparc__
#define isSparky 1
#else 
#define isSparky 0
#endif


char filename[256];
/*char* outfile;*/
int fd, outFd = 0;
FILE *outfp;
FILE *fp;
endianness source;
endianness conversion;
endianness outSource;
char* endian_convert;
int hFlag, vFlag, outFlag, bomFlag, memFlag, emptyFile, writeFlag, printToStdout = 0;
float asciiCounter, surCounter = 0;
int counter, glyphCounter = 0;
float* readTime;
float* writeTime;
float* convertTime;
float* ret;
Glyph* glyph;
ino_t inputINO, outputINO;

static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;



int main(int argc, char** argv)
{
	unsigned char buf[MAX_BYTES] = {0}; 
	unsigned char leBom[] = {0xff, 0xfe};
	unsigned char beBom[] = {0xfe, 0xff};
	/*unsigned char newLineBE[] = {0, '\n'};
	unsigned char newLineLE[] = {'\n', 0};*/
	int rv = 0; 
	glyph = calloc(1, sizeof(Glyph));
	readTime = calloc(3, sizeof(long double) * 3);
	writeTime = calloc(3, sizeof(long double) * 3);
	convertTime = calloc(3, sizeof(long double) * 3);
	ret = calloc(3, sizeof(float) * 3);

	/* After calling parse_args(), filename and conversion should be set. */
	parse_args(argc, argv);

	if (printToStdout == 0 && outFlag){
		if (emptyFile == 0){
		/*if output file is not empty, get BOM of output file*/
			if ((rv = read(outFd, &buf[0], 1)) == 1 && (rv = read(outFd, &buf[1], 1)) == 1){
				if(buf[0] == 255 && buf[1] == 254){
					outSource = LITTLE;



				} else if(buf[0] == 254 && buf[1] == 255){
					outSource = BIG;


				}else{
					print_help();
					quit_converter(outFd, 1);
				}

			}


			if (outSource == LITTLE){
				glyph -> bytes[0] = '\n';
				glyph -> bytes[1] = 0;
				write_glyph(glyph);
				writeFlag = 1;
			}else if (outSource == BIG){
				glyph -> bytes[0] = 0;
				glyph ->bytes[1] = '\n';
				write_glyph(glyph);
				writeFlag = 1;
			}
			else{
				print_help();
				quit_converter(NO_FD, 1);
			}



		}

	}




	if (inputINO == outputINO){
		print_help();
		quit_converter(fd, 1);
	}



	start_clock();
	while((rv = read(fd, &buf[0], 1)) == 1 && (rv = read(fd, &buf[1], 1)) == 1){ 
		end_clock();
		readTime[0] += ret[0];
		readTime[1] += ret[1];
		readTime[2] += ret[2];
			/* 255 = FF  254 = FE
				239 = EF 	BB*/
		if (counter == 0){
			if(buf[0] == 255 && buf[1] == 254){

				source = LITTLE;
				fill_glyph(glyph, buf, source, fd);
				if (bomFlag == 0){
					if (conversion!=LITTLE){
						swap_endianness(glyph);
					}

					write_glyph(glyph);
				}




			} else if(buf[0] == 254 && buf[1] == 255){
				source = BIG;
				fill_glyph(glyph, buf, source, fd);
				if (bomFlag == 0){
					if(conversion!=LITTLE){
						swap_endianness(glyph);
					}

					write_glyph(glyph);

				}



			}else if (buf[0] == 239 && buf[1] == 187){
				if (read(fd, &buf[2], 1) == 1){
					if (buf[2] == 191){
						source = EIGHT;
						break;
					}
				}


			} else {
			/*file has no BOM*/
				print_help();
				quit_converter(fd, 1); 
			}

		}

		else if (counter >= 1 && rv != 0){

			fill_glyph(glyph, buf, source, fd);
			if(conversion!=LITTLE){
				swap_endianness(glyph);
			}
			write_glyph(glyph);


		}

		start_clock();
		counter++;
	}

	end_clock();
	readTime[0] += ret[0];
	readTime[1] += ret[1];
	readTime[2] += ret[2];

	if (source == EIGHT){

		if (conversion == LITTLE){
			if (bomFlag == 0){
				fill_glyph(glyph, leBom, LITTLE, fd);
				write_glyph(glyph);
			}

		}
		else{
			if (bomFlag == 0){
				fill_glyph(glyph, beBom, BIG, fd);
				write_glyph(glyph);
			}
		}
		start_clock();
		while ((rv = read(fd, &buf[0], 1) == 1)){
			end_clock();
			readTime[0] += ret[0];
			readTime[1] += ret[1];
			readTime[2] += ret[2];

			fill_utf8_glyph(glyph, buf,fd);
			write_glyph(glyph);

			start_clock();

		}
		end_clock();
		readTime[0] += ret[0];
		readTime[1] += ret[1];
		readTime[2] += ret[2];
	}



	if (vFlag >= 1)
		print_verbose();



	quit_converter(fd, 0);
	return EXIT_SUCCESS;
}


Glyph* swap_endianness(Glyph* glyph) {
	/* Use XOR to be more efficient with how we swap values. */
	unsigned char temp = 0;
	start_clock();

	temp = glyph -> bytes[1];
	glyph->bytes[1] = glyph->bytes[0];
	glyph->bytes[0] = temp;

	if(glyph->surrogate){  /* If a surrogate pair, swap the next two bytes. */
	temp = glyph -> bytes[3];
	glyph->bytes[3] = glyph->bytes[2];
	glyph->bytes[2] = temp;
}
end_clock();
convertTime[0] += ret[0];
convertTime[1] += ret[1];
convertTime[2] += ret[2];

glyph->end = conversion;
return glyph;
}


Glyph* fill_utf8_glyph(Glyph* glyph, unsigned char data[], int fd){
	unsigned int byte1, byte2, byte3, byte4, cByte1, cByte2, temp, temp2, halfSur11, halfSur12, halfSur21, halfSur22 = 0;
	unsigned int codePoint, msb, cpPrime, surByte1, surByte2 = 0;
	long multiByte = 0;

	/* 1byte */
	if (data[0] < 128){
		asciiCounter++;
		start_clock();
		if (conversion == LITTLE){
			glyph -> bytes[0] = data[0];
			glyph -> bytes[1] = 0;
			glyph -> bytes[2] = 0;
			glyph -> bytes[3] = 0;
			glyph -> surrogate =false;
			glyph -> end = conversion;

			end_clock();
			convertTime[0] += ret[0];
			convertTime[1] += ret[1];
			convertTime[2] += ret[2];
		}
		else {
			glyph -> bytes[0] = 0;
			glyph -> bytes[1] = data[0];
			glyph -> bytes[2] = 0;
			glyph -> bytes[3] = 0;
			glyph -> surrogate =false;
			glyph -> end = conversion;
			end_clock();
			convertTime[0] += ret[0];
			convertTime[1] += ret[1];
			convertTime[2] += ret[2];
		}

		
		
	}
	/* 2 byte */
	else if (data[0] >= 192 && data[0] < 224) {
		byte1 = data[0] & 31;
		byte1 = byte1 << 6;

		if (read(fd, &data[SECOND], 1) == 1){
			byte2 = data[1] & 63;
			codePoint = byte1 + byte2;
		}
		

		start_clock();
		if (conversion == LITTLE){
			glyph -> bytes[0] = codePoint;
			glyph -> bytes[1] = 0;
			glyph -> bytes[2] = 0;
			glyph -> bytes[3] = 0;
			glyph -> surrogate = false;
			glyph -> end = conversion;
			end_clock();
			convertTime[0] += ret[0];
			convertTime[1] += ret[1];
			convertTime[2] += ret[2];

		}else{
			glyph -> bytes[0] = 0;
			glyph -> bytes[1] = codePoint;
			glyph -> bytes[2] = 0;
			glyph -> bytes[3] = 0;
			glyph -> surrogate = false;
			glyph -> end = conversion;
			end_clock();
			convertTime[0] += ret[0];
			convertTime[1] += ret[1];
			convertTime[2] += ret[2];
		}


	}
	/* 3 bytes */
	else if (data[0] >= 224 && data[0] < 240){
		byte1 = data[0] & 15;
		multiByte += byte1;

		multiByte = multiByte << 6;

		if (read(fd, &data[SECOND], 1) == 1){
			byte2 = data[SECOND] & 63;
			multiByte += byte2;
		}

		
		multiByte = multiByte << 6;

		if (read(fd, &data[THIRD], 1) == 1){
			byte3 = data[THIRD] & 63;
			multiByte += byte3;
		}

		codePoint = multiByte;

		cByte1 = multiByte >> 8;
		cByte2 = multiByte & 0xFF;

		start_clock();
		if (conversion == LITTLE){
			glyph -> bytes[0] = cByte2;
			glyph -> bytes[1] = cByte1;
			glyph -> bytes[2] = 0;
			glyph -> bytes[3] = 0;
			glyph -> surrogate = false;
			glyph -> end = conversion;

			end_clock();
			convertTime[0] += ret[0];
			convertTime[1] += ret[1];
			convertTime[2] += ret[2];

		}else{
			glyph -> bytes[0] = cByte1;
			glyph -> bytes[1] = cByte2;
			glyph -> bytes[2] = 0;
			glyph -> bytes[3] = 0;
			glyph -> surrogate = false;
			glyph -> end = conversion;

			end_clock();
			convertTime[0] += ret[0];
			convertTime[1] += ret[1];
			convertTime[2] += ret[2];

		}

	}

	/* 4 bytes */
	else if (data[0] >= 240 && data[0] < 248){
		byte1 = data[0] & 7;
		multiByte += byte1;

		if (read(fd, &data[SECOND], 1) == 1){
			byte2 = data[SECOND] & 63;
			multiByte += byte2;
		}

		
		multiByte = multiByte << 6;

		if (read(fd, &data[THIRD], 1) == 1){
			byte3 = data[THIRD] & 63;
			multiByte += byte3;
		}

		multiByte = multiByte << 6;

		if (read(fd, &data[FOURTH], 1) == 1){
			byte4 = data[FOURTH] & 63;
			multiByte += byte4;
		}

		codePoint = multiByte;
		/*subtract 0x10000*/
		cpPrime = multiByte - 0x10000;
		msb = cpPrime >> 10;
		cpPrime = cpPrime & 0x3FF;

		surByte1 = 0xD800 + msb;
		surByte2 = 0xDC00 + cpPrime;


		temp = surByte1;
		temp2 = surByte2;

		halfSur11 = surByte1 >> 8;
		halfSur12 = temp & 0xFF;

		halfSur21 = surByte2 >> 8;
		halfSur22 = temp2 & 0xFF;
		/*need to split the surrogate pair into 2 separate bytes*/
		start_clock();
		if (conversion == LITTLE){
			glyph -> bytes[0] = halfSur12;
			glyph -> bytes[1] = halfSur11;
			glyph -> bytes[2] = halfSur22;
			glyph -> bytes[3] = halfSur21;
			glyph -> surrogate = true;
			glyph -> end = conversion;
			surCounter++;

			end_clock();
			convertTime[0] += ret[0];
			convertTime[1] += ret[1];
			convertTime[2] += ret[2];
		}else {
			glyph -> bytes[0] = halfSur11;
			glyph -> bytes[1] = halfSur12;
			glyph -> bytes[2] = halfSur21;
			glyph -> bytes[3] = halfSur22;
			glyph -> surrogate = true;
			glyph -> end = conversion;
			surCounter++;
			end_clock();
			convertTime[0] += ret[0];
			convertTime[1] += ret[1];
			convertTime[2] += ret[2];
		}

	}
	glyphCounter++;
	
	return glyph;


}


Glyph* fill_glyph(Glyph* glyph, unsigned char data[], endianness end, int fd){

	unsigned int bits = 0;

	if (end == BIG && source != EIGHT){
		glyph->bytes[0] = data[1];
		glyph->bytes[1] = data[0];
		if (data[0] == 0 && data[1] < 128){
			asciiCounter++;
		}
		bits |= ((data[FIRST]<<8) + (data[SECOND]));
	}

	else if (end == LITTLE){
		glyph->bytes[0] = data[0];
		glyph->bytes[1] = data[1];
		if (data[0] < 128 && data[1] == 0){
			asciiCounter++;
		}
		bits |= ((data[FIRST]) + (data[SECOND]<<8));
	}

	else if (end == BIG && source == EIGHT){
		glyph -> bytes[0] = data[0];
		glyph -> bytes[1] = data[1];
	}
	
	

	/*fprintf(stderr, "bits: %08x\n", bits);*/
	
	/* Check high surrogate pair using its special value range.*/
	if(bits > 0xD800 && bits < 0xDBFF){ 
		start_clock();
		if(read(fd, &data[FIRST], 1) == 1 && read(fd, &data[SECOND], 1) == 1){
			end_clock();
			readTime[0] += ret[0];
			readTime[1] += ret[1];
			readTime[2] += ret[2];

			bits = 0;
			if (end == BIG){
				bits |= ((data[FIRST] << 8) + (data[SECOND]));
			}else if (end == LITTLE){
				bits |= ((data[FIRST]) + (data[SECOND]<<8));
			}
			
			if(bits > 0xDC00 && bits < 0xDFFF){ /* Check low surrogate pair.*/
			glyph->surrogate = true;
			surCounter++;
		} else {
			glyph -> surrogate = false;
		}
	}
}
else {
	glyph -> surrogate = false; 
}
if(!glyph->surrogate){
	glyph->bytes[THIRD] = '\0';
	glyph->bytes[FOURTH] = '\0';
} else {
	if (end == LITTLE){
		glyph->bytes[THIRD] = data[FIRST]; 
		glyph->bytes[FOURTH] = data[SECOND];

	}else{
		glyph->bytes[THIRD] = data[SECOND]; 
		glyph->bytes[FOURTH] = data[FIRST];
	}

}
glyph->end = end;

glyphCounter++;

return glyph;

}




void write_glyph(Glyph* glyph){
	start_clock();
	if (outFlag){
		if (printToStdout){
			if(glyph->surrogate){
				write(STDOUT_FILENO, glyph->bytes, SURROGATE_SIZE);
			} else {
				write(STDOUT_FILENO, glyph->bytes, NON_SURROGATE_SIZE);
			}

		}
		else if (conversion != outSource && bomFlag == 1 && writeFlag == 1){
			print_help();
			quit_converter(fd, 1);

		}
		else{
			if (glyph->surrogate){
				write(outFd, glyph->bytes, SURROGATE_SIZE);
			}
			else{
				write(outFd, glyph->bytes, NON_SURROGATE_SIZE);
			}
		}
		
	}
	else{
		if(glyph->surrogate){
			write(STDOUT_FILENO, glyph->bytes, SURROGATE_SIZE);
		} else {
			write(STDOUT_FILENO, glyph->bytes, NON_SURROGATE_SIZE);
		}

	}

	end_clock();
	writeTime[0]+=ret[0];
	writeTime[1]+=ret[1];
	writeTime[2]+=ret[2];
	
	
}

void print_verbose(){
	off_t size;
	char buf[PATH_MAX + 1];
	char *absolutePath;
	char *utf16be = "UTF-16BE";
	char *utf16le = "UTF-16LE";
	char *utf8 = "UTF-8";
	char hostname[128];
	struct utsname system;
	float asciiPercent, surPercent = 0;
	float fileSize = 0;


	struct stat st;
	stat(filename, &st);
	size = st.st_size;

	fileSize = size/1000.0;

	fprintf(stderr, "\n\nInput file size: %1.3f kb\n", fileSize);

	
	absolutePath = realpath(filename, buf);
	if (absolutePath) {
		fprintf(stderr, "Input file path: %s\n", buf);
	} else {
		print_help();
		quit_converter(fd, 1);
	}

    /*print input file encoding*/
	if (source == BIG){
		fprintf(stderr,"Input file encoding: %s\n", utf16be);
	}else if (source == LITTLE){
		fprintf(stderr, "Input file encoding: %s\n", utf16le);
	}else if (source == EIGHT){
		fprintf(stderr, "Input file encoding: %s\n", utf8);
	}

	else{
		fprintf(stderr,"Invalid encoding\n");
	}

    /*print output file encoding*/
	if (conversion == BIG){
		fprintf(stderr,"Output encoding: %s\n", utf16be);
	}
	else if (conversion == LITTLE){
		fprintf(stderr,"Output encoding: %s\n", utf16le);
	}
	else{
		print_help();
		quit_converter(fd, 1);
	}

	gethostname(hostname, sizeof(hostname));
	fprintf(stderr, "Hostmachine: %s\n", hostname);

	uname(&system);
	fprintf(stderr, "Operating System: %s\n", system.sysname);

	if (vFlag > 1){
		/*time*/
		fprintf(stderr, "Reading: real = %.1f, user = %.1f, sys = %.1f\n", readTime[0], readTime[1], readTime[2]);
		fprintf(stderr, "Converting: real = %.1f, user = %.1f, sys = %.1f\n", convertTime[0], convertTime[1], convertTime[2]);
		fprintf(stderr, "Writing: real = %.1f, user = %.1f, sys = %.1f\n", writeTime[0], writeTime[1], writeTime[2]);

		asciiPercent = ((asciiCounter / glyphCounter) * 100);
		asciiPercent = round(asciiPercent);


		fprintf(stderr, "ASCII: %d%%\n", (int)asciiPercent);

		surPercent = ((surCounter / glyphCounter) * 100);
		surPercent = round(surPercent);

		fprintf(stderr, "Surrogates: %d%%\n", (int)surPercent);

		fprintf(stderr, "Glyphs: %d\n", glyphCounter);

	}


	

}

void start_clock(){
	st_time = times(&st_cpu);
}


float* end_clock(){

	en_time = times(&en_cpu);

	ret[0] = (float)(en_time - st_time) /  sysconf(_SC_CLK_TCK); /*real time*/
	ret[1] = (float)(en_cpu.tms_utime - st_cpu.tms_utime) /  sysconf(_SC_CLK_TCK); /*user time*/
	ret[2] = (float)(en_cpu.tms_stime - st_cpu.tms_stime) /  sysconf(_SC_CLK_TCK); /*sys time*/


	return ret;

}

void parse_args(int argc, char**argv){
	char c = 0;
	char outfile[256];
	int i = 0;
	struct stat buf;

	static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"UTF=", required_argument, 0, 'u'},

		{0, 0, 0, 0}
	};

	
	
	while((c = getopt_long(argc, argv, "hvu:", long_options, &optind)) != -1){
		
		switch(c){ 

			
			case 'u':
			endian_convert = optarg;
			break;

			case 'h':
			hFlag = 1;
			break;

			case'v':
			vFlag++;
			break;


			default:
			print_help();
			quit_converter(NO_FD, 1);
			break;

		}


	}

	if (hFlag == 1 && optarg == NULL){
		print_help();
		quit_converter(NO_FD, 0);
	}else if (hFlag == 1 && optarg != NULL){
		print_help();
		quit_converter(NO_FD, 1);
	}


	for (i = 0; i < argc; i++){
		if (strcmp("--UTF", argv[i]) == 0){
			print_help();
			quit_converter(NO_FD, 1);
		}
	}



	/*so -u 16BE counts at 6 argcs. --UTF=16BE counts as 5 args.*/
	if(optind < argc && hFlag != 1){
		
		strcpy(filename, argv[optind]);

		/*check if input file is empty */
		if (!(fp = fopen(filename, "a+"))){
			print_help();
			quit_converter(NO_FD, 1);
		}


		fseek(fp, 0, SEEK_END); 
		if(ftell(fp) == 0){
			print_help();
			quit_converter(NO_FD, 1);

		}

		fclose(fp);


		/*open input file */
		fd = open(filename, O_RDONLY);
		if (fd < 0){
			print_help();
			quit_converter(fd, 1);
		}

		stat(filename, &buf);
		inputINO = buf.st_ino;

		if ((optind + 1) < argc){
			
			strcpy(outfile, argv[optind + 1]);

			if (strcmp(outfile, "stdout") == 0){
				printToStdout = 1;
			}

			outFlag = 1;

			if (optind + 2 < argc){
				print_help();
				quit_converter(fd, 1);
			}
			
			
			/*if not printing to stdout, check the length of output file*/
			if (printToStdout == 0){
				if (!(outfp = fopen(outfile, "a+"))){
					print_help();
					quit_converter(fd, 1);
				}


				if (outfp !=NULL){
					fseek(outfp, 0, SEEK_END);
					if(ftell(outfp) == 0){
						emptyFile = 1;
					}else{
						bomFlag = 1;

					}


					fclose(outfp);

				}

				outFd = open(outfile, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
				stat(outfile, &buf);
				outputINO = buf.st_ino;

			}
			

		}

		
	} else {
		print_help();
		quit_converter(NO_FD, 1);
	}


	if(endian_convert == NULL){
		print_help();
		quit_converter(fd, 1);
	}

	if(strcmp(endian_convert, "16LE") == 0){ 
		conversion = LITTLE;
	} else if(strcmp(endian_convert, "16BE") == 0){
		conversion = BIG;
	} else {
		print_help();
		quit_converter(fd, 1);
	}

}

void print_help () {

	fprintf(stderr, "%s", USAGE);
	
}

int quit_converter(int fd, int exitStatus){

	free(glyph);
	free(readTime);
	free(writeTime);
	free(convertTime);
	free(ret);

	close(STDERR_FILENO);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(outFd);

	
	if(fd != NO_FD)
		close(fd);

	if (exitStatus == 0)
		exit(EXIT_SUCCESS);
	
	exit(EXIT_FAILURE);
	
}
