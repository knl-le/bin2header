#include <algorithm> // transform
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
using namespace std;

#include <cstdio>


const string version = "0.1.1";

#define len(a) (sizeof(a)/sizeof(*a))


/** Normalizes the path for they current system */
string NormalizePath(string path) {
	string new_path = path;
	string to_replace;
	string replace_with;

#ifdef __WIN32__
	to_replace = '/';
	replace_with = '\\';
#else
	to_replace = '\\';
	replace_with = '/';
#endif

	string cur_char;
	for (int idx = 0; idx < path.length(); idx++) {
		cur_char = path[idx];
		if (cur_char == to_replace) {
			new_path.replace(idx, 1, replace_with);
		}
	}

#ifdef __WIN32__
	// MSYS2/MinGW paths
	string first_chars = new_path.substr(0, 3);
	// FIXME: use tolower from cstio?
	transform(first_chars.begin(), first_chars.end(), first_chars.begin(), ::tolower);

	// TODO: support all drive letters
	if (first_chars == "\\c\\") {
		new_path.replace(0, 2, "C:");
	}
#endif

	return new_path;
}


string JoinPath(string a, string b) {
#ifdef __WIN32__
	return a.append("\\").append(b);
#else
	return a.append("/").append(b);
#endif
}


string GetBaseName(string f) {
	int split;

#ifdef __WIN32__
	split = f.find_last_of('\\');
#else
	split = f.find_last_of('/');
#endif
	f.erase(0, split + 1);

	return f;
}


string GetDirName(string f) {
	int split;

#ifdef __WIN32__
	split = f.find_last_of('\\');
#else
	split = f.find_last_of('/');
#endif
	f.erase(split, f.length() - split);

	return f;
}


int PrintUsage(string exe="bin2header") {
	cout << "\nbin2header version " << version << endl;
	cout << "2019 Jordan Irwin <antumdeluge@gmail.com>\n\n\tUsage:\t" << exe << " <file>\n" << endl;
	return 1;
}


int main(int argc, char** argv) {
	string executable = NormalizePath(argv[0]);
	executable = GetBaseName(executable);

	if (argc < 2) {
		cout << "\nERROR: Missing <file> argument\n";
		return PrintUsage(executable);
	}

	string source_file = NormalizePath(argv[1]);

	// Check if file exists
	FILE* test;
	test = fopen(source_file.c_str(), "r");
	fclose(test);

	if (!test) {
		cout << "\nFile: \"" << source_file << "\" does not exist\n";
		return PrintUsage(executable);
	}

	/* Get Filenames and Path */
	string filename = GetBaseName(source_file);
	string hname = filename;

	/* START Remove Unwanted Characters */
	int hname_len = hname.length();
	char badchars[6] = {'\\', '+', '-', '*', ' '};
	int current;
	int x;
	for (current = 0; current < hname_len; current++) {
		for (x = 0; x < len(badchars); x++) {
			if ((hname[current] == badchars[x]) or (hname[current] == '.'))
				hname.replace(current, 1, "_");
			if (filename[current] == badchars[x])
				filename.replace(current, 1, "_");
		}
	}
	/* END Remove Unwanted Characters */

	filename += ".h";

	/* START Uppercase Name for Header */
	char hname_upper[hname_len + 2];
	for (current = 0; current < len(hname_upper); current++) {
		hname_upper[current] = hname[current];
		hname_upper[current] = toupper(hname_upper[current]);
	}

	string name_upper_h = hname_upper;
	name_upper_h.append("_H");
	/* END Uppercase Name for Header */

	/* START Read Data In */
	ifstream infile;
	infile.open(source_file.c_str(), ifstream::binary);

	int data_length;
	infile.seekg(0, ifstream::end);
	data_length = infile.tellg();
	infile.seekg(0, ifstream::beg);

	char data[data_length];

	infile.read(data, data_length);
	infile.close();
	/* END Read Data In */

	/* START Read Data Out to Header */
	ofstream outfile(filename.c_str());
	outfile << "#ifndef " << name_upper_h.c_str() << "\n#define " << name_upper_h.c_str() << "\n\nstatic const unsigned char " << hname << "[] = {\n";

	for (current = 0; current < data_length; current++) {
		if ((current % 12) == 0) outfile << "    ";

		stringstream ss;
		ss << "0x" << hex << setw(2) << setfill('0') << (int) (unsigned char) data[current];
		outfile << ss.str();

		if ((current + 1) < data_length) outfile << ", ";

		if ((current % 12) == 11) outfile << "\n";
	}

	outfile << "\n};\n\n#endif /* " << name_upper_h << " */\n";
	outfile.close();
	/* END Read Data Out to Header */

	cout << "Exported to: " << filename << endl;

	return 0;
}
