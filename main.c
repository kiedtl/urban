//
// urban: a optimizing Brainfsck compiler
// (c) Kied Llaentenn
// See the LICENSE for more information
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void usage ( void );
void print_command ( int count, char i );

int
main ( int argc, char *argv[]  )
{
	if (argc < 2) usage();

	char *path = (char*) malloc(strlen(argv[1]) * sizeof(char));
	strcpy(path, argv[1]);

	// check if file exists
	struct stat stats;
	if (stat(path, &stats) != 0)
	{
		fprintf(stderr, "urban: cannot stat %s: ", path);
		perror("stat()");
		return 1;
	}

	// read file into buffer
	FILE *file = fopen(path, "r");
	unsigned long flen = 0;
	char *buffer;

	if (file == NULL)
	{
		fprintf(stderr, "urban: cannot open %s: ", path);
		perror("fopen()");
		return 1;
	}

	fseek(file, 0, SEEK_END);
	flen = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = (char*) malloc(sizeof(char) * flen);
	fread(buffer, sizeof(buffer), flen, file);
	fclose(file);

	fprintf(stdout, "#include <stdio.h>\nint \nmain( void )\n{\n");
	fprintf(stdout, "char buf[30000] = {0};\n");
	fprintf(stdout, "char *p = buf;\n");

	// iter over buffer, reading it into an array of Instructions
	// alloc 1 instructions, realloc as needed
	int ctr = 0;
	int i = 0;
	for (; buffer[i] != 0; i++)
	{
		int ins_ctr = 1;
		char orig = buffer[i];

		// comments
		if (buffer[i] == ';')
		{
			i--;
			if (buffer[i] != '\\') // escape
			{
				i++;
				while (buffer[i] != '\n') {
					i++;
					orig = buffer[i];
				}
				continue;
			}
			i++;
		}

		// compress multiple instructions into one
		if (buffer[i + 1] == buffer[i])
		{
			while (buffer[i] != orig)
			{
				fprintf(stderr, "found dup instruction %c\n", buffer[i]);

				i += 1;
				ins_ctr += 1;
			}
		}

		fprintf(stderr, "found command '%c', of count %i\n", buffer[i], ins_ctr);
		print_command(ins_ctr, buffer[i]);
		continue;
	}

	fprintf(stdout, "}\n");

	// cleanup
	//if (buffer) free(buffer);
	free(path);
}

void
usage ( void )
{
	fprintf(stderr, "usage: urban [file]\n");
	exit(1);
}


void
print_command ( int count, char i )
{
	switch (i)
	{
		case '^':
			fprintf(stdout, "p = 0;\n");
			break;
		case '>':
			fprintf(stdout, "p += %i;\n", count);
			break;
		case '<':
			fprintf(stdout, "p -= %i;\n", count);
			break;
		case '[':
			fprintf(stdout, "while (*p) {\n");
			break;
		case ']':
			fprintf(stdout, "}\n");
			break;
		case '+':
			fprintf(stdout, "*p += %i;\n", count);
			break;
		case '-':
			fprintf(stdout, "*p -= %i;\n", count);
			break;
		case '.':
			fprintf(stdout, "putchar(*p);\n");
			break;
		case '&':
			fprintf(stdout, "fprintf(stderr, \"%%c\", *p);\n");
			break;
		case ',':
			fprintf(stdout, "*p = getchar();\n");
			break;
		case '*':
			fprintf(stdout, "*p = 0;\n");
			break;
		case ';':
			break;
		case '%':
			fprintf(stdout, "exit((int)*p);\n");
			break;
		default: break;
	}
}
