const fs = require("fs");
const { exit, argv } = require("process");

const yellow = "\033[0;33m"
const green = "\033[0;32m"
const reset = "\033[00m"

let arg = ""

if (argv.length < 3) {
	process.stderr.write("Error: missing argument. Please specify either XID_START or XID_CONTINUE.\n");
	exit(1);
}
switch (argv[2].split(" ").join("")) {
	case "XID_START":
		arg = "XID_START"
		process.stdout.write(`${green}Generating XID_START.c...\n`)
		break
	case "XID_CONTINUE":
		arg = "XID_CONTINUE"
		process.stdout.write(`${yellow}Generating XID_CONTINUE.c...\n`)
		break
	default:
		process.stderr.write(`Error: unrecognised option '${argv[2]}'. Please specify XID_START or XID_CONTINUE.\n`);
		exit(1);
}

let rawdata = fs.readFileSync(`${arg}.txt`).toString().split("\n")
for (let line in rawdata) rawdata[line] = rawdata[line].slice(0, rawdata[line].indexOf(";")).split(" ").join("")

let cnt = 0;

let set = new Set()
let pre = `#define MAX_${arg} (0x3134A)\n\n// clang-format off\nstatic const unsigned char ${arg}_TABLE[] = { // `
let out = ""

const put = key => {
	cnt++
	process.stdout.write("\rIndexing code points: " + cnt.toString())
	set.add(key)
};

const print = key => {
	cnt++
	process.stdout.write("\rGenerating lookup table: " + cnt.toString())
	out += key
};

for (let line of rawdata) {
	if (line.includes("..")) {
		let both = line.split("..");
		let first = Number("0x" + both[0])
		let second = Number("0x" + both[1])
		while (first <= second) put(first++)
	} else put(Number("0x" + line))
}

cnt = 0
process.stdout.write("\n")
let tmp = ""
for (let i = 0; i <= 0x3134A; i++) {
	if (i > 0 && (i % 8) == 0) {
		print("0b" + tmp + ", ")
		if (i > 0 && (i % 64) == 0) out += "\n"
		tmp = ""
	}
	tmp = (set.delete(i) ? "1" : "0") + tmp
}
pre += cnt + "\n"
fs.writeFileSync(`${arg}.c`, pre)
fs.appendFileSync(`${arg}.c`, out + "\n};\n// clang-format on\n")

process.stdout.write(`${reset}\n`)
