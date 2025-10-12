#include <crypt.h>
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "color.h"
#include "torrentfile.h"

// Parse command-line arguments for input/output file and piece size
static void parse_args(int argc, char* argv[], char** input_file, char** output_file, uint32_t* piece_size) {
	int opt;
	while ((opt = getopt(argc, argv, "f:o:p:h")) != -1) {
		switch (opt) {
		case 'f':
			if (!optarg) err(EXIT_FAILURE, "input_file missing.\n");
			*input_file = optarg;
			break;
		case 'o':
			if (!optarg) err(EXIT_FAILURE, "output_file missing.\n");
			*output_file = optarg;
			break;
		case 'p':
			if (!optarg) err(EXIT_FAILURE, "piece_size missing.\n");
			*piece_size = (uint32_t)atoi(optarg);
			if (*piece_size == 0) err(EXIT_FAILURE, "piece_size is invalid.\n");
			break;
		case 'h':
			fprintf(stderr, "Usage: %s [-f input_file] [-o output_file] [-p piece_size]\n", argv[0]);
			exit(EXIT_SUCCESS);
		}
	}
}

// Generate output file name by appending .torrent to input file base name
static char* output_file_name(const char* input_file) {
	size_t len = strlen(input_file);
	const char* dot = strrchr(input_file, '.');
	size_t base_len = dot ? (size_t)(dot - input_file) : len;
	char* output_file = malloc(base_len + 9); // ".torrent" + null
	if (!output_file) {
		fprintf(stderr, RED "Error: Memory allocation failed\n" RESET);
		exit(EXIT_FAILURE);
	}
	strncpy(output_file, input_file, base_len);
	output_file[base_len] = '\0';
	strcat(output_file, ".torrent");
	return output_file;
}

// Get the size of a file in bytes
static uint32_t get_file_size(const char* filename) {
	struct stat sb;
	if (stat(filename, &sb) == -1) {
		fprintf(stderr, RED "Error: Could not get file size for '%s': %s\n" RESET, filename, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (!S_ISREG(sb.st_mode)) {
		fprintf(stderr, RED "Error: '%s' is not a regular file.\n" RESET, filename);
		exit(EXIT_FAILURE);
	}
	return (uint32_t)sb.st_size;
}

// Compute MD5 hash of a memory chunk
static void md5_hash_chunk(void* mem, uint32_t n, unsigned char* digest) {
	MD5((const u_char*)mem, (size_t)n, digest);
}

// Parse and print information from a .torrent file
static void parse(const char* file_name) {
	eltextorrent_file_t torrent = { 0 };
	FILE* fp = fopen(file_name, "rb");
	if (!fp) {
		fprintf(stderr, RED "Error: Could not open file '%s': %s\n" RESET, file_name, strerror(errno));
		exit(EXIT_FAILURE);
	}
	fread(&torrent, sizeof(torrent), 1, fp);

	printf(GRN "Name:" CYN " %s\n" RESET, torrent.name);
	printf(GRN "Length:" CYN " %u bytes\n" RESET, torrent.length);
	printf(GRN "Piece length:" CYN " %u bytes\n" RESET, torrent.piece_length);
	printf(GRN "Infohash:" CYN " [%s]\n" RESET, torrent.infohash);

	fseek(fp, sizeof(torrent), SEEK_SET);
	size_t piece_count = (torrent.length + torrent.piece_length - 1) / torrent.piece_length;
	torrent.pieces = malloc(piece_count * HASH_SIZE + 1);
	if (!torrent.pieces) {
		fprintf(stderr, RED "Error: Memory allocation failed\n" RESET);
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	fread(torrent.pieces, torrent.piece_length, piece_count, fp);
	printf(GRN "Pieces string:" CYN " [%s]\n" RESET, torrent.pieces);
	free(torrent.pieces);
	fclose(fp);
}


int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "C.utf8");

	char* input_file = NULL;
	char* output_file = NULL;
	bool output_file_allocated = false;
	uint32_t piece_length = 0;
	uint32_t piece_count = 0;
	uint32_t file_length = 0;

	// Parse mode: print .torrent file info
	if (argc == 3 && strcmp(argv[1], "parse") == 0) {
		parse(argv[2]);
		return EXIT_SUCCESS;
	}

	// Parse command-line arguments
	parse_args(argc, argv, &input_file, &output_file, &piece_length);
	if (!input_file) {
		fprintf(stderr, RED "Error: Input file is required. Use -h for help\n" RESET);
		exit(EXIT_FAILURE);
	}
	if (!output_file) {
		output_file = output_file_name(input_file);
		output_file_allocated = true;
	}
	if (piece_length == 0) piece_length = 65536u; // 64KB default

	if (access(input_file, F_OK) == -1) {
		fprintf(stderr, RED "Error: Input file '%s' does not exist.\n" RESET, input_file);
		if (output_file_allocated) free(output_file);
		exit(EXIT_FAILURE);
	}

	file_length = get_file_size(input_file);
	piece_count = (file_length + piece_length - 1) / piece_length;

	printf(GRN "Input file: %s\n" RESET, input_file);
	printf(GRN "Input file size: %u bytes\n" RESET, file_length);
	printf(GRN "Output file: %s\n" RESET, output_file);
	printf(GRN "Piece size: %u bytes\n" RESET, piece_length);
	printf(GRN "Piece count: %u\n" RESET, piece_count);

	// Open and memory-map the input file
	int fd = open(input_file, O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, RED "Error: Could not open input file '%s': %s\n" RESET, input_file, strerror(errno));
		if (output_file_allocated) free(output_file);
		exit(EXIT_FAILURE);
	}
	void* map = mmap(NULL, file_length, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	if (map == MAP_FAILED) {
		perror(RED "mmap" RESET);
		if (output_file_allocated) free(output_file);
		exit(EXIT_FAILURE);
	}

	// Allocate memory for piece hashes
	u_char* pieces = malloc(piece_count * HASH_SIZE + 1);
	if (!pieces) {
		fprintf(stderr, RED "Error: Memory allocation failed\n" RESET);
		munmap(map, file_length);
		if (output_file_allocated) free(output_file);
		exit(EXIT_FAILURE);
	}

	// Hash each piece and encode in base64
	u_char digest[MD5_DIGEST_LENGTH];
	u_char base64[HASH_SIZE + 3]; // +2 for safety, +1 for null
	for (uint32_t i = 0; i < piece_count; i++) {
		uint32_t offset = i * piece_length;
		uint32_t chunk_size = (i == piece_count - 1) ? (file_length - offset) : piece_length;
		md5_hash_chunk((u_char*)map + offset, chunk_size, digest);
		int evp_len = EVP_EncodeBlock(base64, digest, MD5_DIGEST_LENGTH);
		base64[HASH_SIZE] = '\0';
		printf(YEL "Piece %4u: [%s] <%d>\n" RESET, i + 1, base64, evp_len);
		memcpy(pieces + i * HASH_SIZE, base64, HASH_SIZE);
	}
	pieces[piece_count * HASH_SIZE] = '\0';
	printf(MAG "Pieces string: [%s]\n" RESET, pieces);
	printf(GRN "All pieces hashed successfully.\n" RESET);

	// Prepare torrent file structure
	eltextorrent_file_t torrent = { 0 };
	strncpy((char*)torrent.name, input_file, NAME_MAX);
	torrent.length = file_length;
	torrent.piece_length = piece_length;
	md5_hash_chunk(pieces, piece_count * HASH_SIZE, digest);
	EVP_EncodeBlock(base64, digest, MD5_DIGEST_LENGTH);
	base64[HASH_SIZE] = '\0';
	strncpy((char*)torrent.infohash, (char*)base64, HASH_SIZE);

	// Write torrent file
	FILE* ofp = fopen(output_file, "wb");
	if (!ofp) {
		fprintf(stderr, RED "Error: Could not open output file '%s': %s\n" RESET, output_file, strerror(errno));
		free(pieces);
		munmap(map, file_length);
		if (output_file_allocated) free(output_file);
		exit(EXIT_FAILURE);
	}
	fwrite(&torrent, sizeof(torrent), 1, ofp);
	fwrite(pieces, 1, piece_count * HASH_SIZE + 1, ofp);
	fclose(ofp);
	printf(GRN "Torrent file '%s' created successfully.\n" RESET, output_file);

	if (output_file_allocated) free(output_file);
	free(pieces);
	munmap(map, file_length);
	return EXIT_SUCCESS;
}
