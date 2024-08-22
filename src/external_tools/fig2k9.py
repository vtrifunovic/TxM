import re

def main(figlet, output):
    f = open(figlet, "r").read()
    g = open(output, "w")
    f = f.split("@@")
    new_space = "$ \n" + "   \n"*2 + "   $$\n\n"
    for idx, l in enumerate(f):
        if idx == 0:
            g.write("# This is a comment line\n")
            g.write(f"# Font was converted over from '{figlet}'\n")
            g.write(f"# Original comments below\n\n\n")
            for x in l.split("\n"):
                g.write("# " + x + "\n")
            g.write("# Font definitions:\n\n")
            g.write(new_space)
            continue
        g.write(f"${chr(idx+32)}\n")
        l = re.sub(r"\$", " ", l)
        l = re.sub("@", "", l)
        l = re.sub("\t", "    ", l)
        l = re.sub(r"^[\n]", "", l)
        g.write(l)
        g.write("$$\n\n")
        print(idx, "\n", l)
        if idx >= 101: # only support up chars 32 - 159
            g.close()
            return


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--figlet", required=True, help="Figlet font to convert")
    parser.add_argument("--output", required=True, help="Name of ouput file")
    args = parser.parse_args()
    main(args.figlet, args.output)