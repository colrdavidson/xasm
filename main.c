#include "minilib.h"

typedef struct {
	u8 byte[15];
	u8 len;
} Inst;

typedef struct {
	u8 *data;
	u64 size;
} Slice;

static Slice to_slice(u8 *data, u64 size) {
	Slice s;
	s.data = data;
	s.size = size;
	return s;
}
#define STR_SLICE(x) to_slice((u8 *)(x), (sizeof(x)-1))
#define SLICE_LIT(x) (int)((x).size), (x).data

#define line_error(p, ...)  do {                                                                                      \
	printf("\e[1;39m%s:%llu:%llu \e[1;31merror:\e[1;39m ", (p)->filename, (p)->line_num + 1, (p)->pos - (p)->line_start); \
	printf(__VA_ARGS__);                                                                                              \
	printf("\e[0m    %.*s\n", SLICE_LIT(get_full_line(p)));                                                           \
	exit(1);                                                                                                          \
} while (0)

static Slice slice_idx(Slice s, u64 idx) {
	if (idx > s.size) {
		panic("Invalid idx %llu:%llu!\n", idx, s.size);
	}

	Slice out;
	out.data = s.data + idx;
	out.size = s.size - idx;
	return out;
}

static bool slice_eq(Slice a, Slice b) {
	if (a.size != b.size) {
		return false;
	}

	for (int i = 0; i < a.size; i += 1) {
		if (a.data[i] != b.data[i]) {
			return false;
		}
	}

	return true;
}

static Slice load_file(char *filename) {
	int fd = open(filename, O_RDONLY);
	if (fd < 0) {
		panic("Failed to open %s\n", filename);
	}

	u64 length = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	u64 aligned_length = round_size(length, 0x1000);
	u8 *data = mmap(NULL, aligned_length, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	return to_slice(data, length);
}

static void hexdump(Slice s) {
	int display_width = 32;

	printf("[");
	int tail = s.size % display_width;
	int trunc_width = s.size - tail;
	for (size_t i = 0; i < trunc_width; i += display_width) {
		int j = 0;
		for (; j < (display_width - 2); j += 2) {
			printf("%02x%02x ", s.data[i+j], s.data[i+j+1]);
		}

		if (i + display_width == s.size) {
			printf("%02x%02x ", s.data[i+j], s.data[i+j+1]);
		} else {
			printf("%02x%02x\n", s.data[i+j], s.data[i+j+1]);
		}
	}

	if (tail) {
		int j = trunc_width;
		for (; j < (s.size - 2); j += 2) {
			printf("%02x%02x ", s.data[j], s.data[j+1]);
		}

		int rem = s.size - j;
		if (rem == 2) {
			printf("%02x%02x", s.data[j], s.data[j+1]);
		} else if (rem == 1) {
			printf("%02x", s.data[j]);
		}
	}
	printf("]\n");
}

typedef struct {
	u64 pos;
	u64 line_num;
	u64 line_start;
	char *filename;
	Slice file;
} ParseState;

static bool is_space(char c) {
	return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

static void next_ch(ParseState *p) {
	p->pos += 1;
	if (p->file.data[p->pos] != '\n') {
		return;
	}

	p->line_num += 1;
	p->line_start = p->pos + 1;
}

static Slice get_full_line(ParseState *p) {
	u64 end_pos = p->line_start;
	for (u64 i = p->line_start; i < p->file.size; i += 1) {
		if (p->file.data[i] == '\n') {
			end_pos = i;
			break;
		}
	}

	return to_slice(p->file.data + p->line_start, end_pos - p->line_start);
}

static void eat_spaces(ParseState *p) {
	while (p->pos < p->file.size) {
		char c = p->file.data[p->pos];
		if (!is_space(c)) { return; }
		next_ch(p);
	}
}

static Slice get_token(ParseState *p) {
	u64 start = p->pos;
	while (p->pos < p->file.size) {
		char c = p->file.data[p->pos];
		if (is_space(c)) { 
			break;
		}

		next_ch(p);
	}

	return to_slice(p->file.data + start, p->pos - start);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		panic("%s: expected a file to assemble!\n", argv[0]);
	}

	ParseState p = { 
		.pos = 0, 
		.line_num = 0, 
		.line_start = 0, 
		.filename = argv[1], 
		.file = load_file(argv[1]) 
	};

	DynArr(Inst) insts;
	dyn_init(insts, 16);

	while (p.pos < p.file.size) {
		eat_spaces(&p);
		if (p.pos >= p.file.size) {
			break;
		}

		Slice token = get_token(&p);

		// This is a label
		if (token.data[token.size - 1] == ':') {
			token.size -= 1;

			printf("Got label: %.*s\n", SLICE_LIT(token));
		} else if (slice_eq(token, STR_SLICE("extern"))) {
			eat_spaces(&p);
			Slice label = get_token(&p);
			if (label.data[label.size - 1] != ':') {
				line_error(&p, "invalid label: %.*s\n", SLICE_LIT(label));
			}
			label.size -= 1;

			printf("Got extern label: %.*s\n", SLICE_LIT(label));
		} else if (slice_eq(token, STR_SLICE("mov"))) {
			eat_spaces(&p);

			Slice arg1 = get_token(&p);
			if (arg1.data[arg1.size - 1] == ',') {
				arg1.size -= 1;
			}

			eat_spaces(&p);
			Slice arg2 = get_token(&p);

			printf("got mov %.*s, %.*s\n", SLICE_LIT(arg1), SLICE_LIT(arg2));
		} else if (slice_eq(token, STR_SLICE("syscall"))) {
			printf("got syscall\n");
		} else {
			line_error(&p, "unknown token: \"%.*s\"\n", SLICE_LIT(token));
		}
	}

	return 0;
}
