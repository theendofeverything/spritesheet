/* *************DOC***************
 * Parse the gcc preprocessor header list (cflag -M) to make a
 * list of header paths for ctags to use.
 *
 * Example
 * -------
 * Use gcc to output a list of headers in headers-M.txt:
 * $(CC) $(CFLAGS) $< -M > headers-M.txt
 *
 * List all headers in new file headers.txt:
 * ./parse-headers.exe
 *
 * Only list local headers (my libs) in new file headers.txt:
 * ./parse-headers.exe M
 *
 * Arguments
 * ---------
 * None : include all libs
 * M : only include my libs (exclude system libs)
 *
 * Strings -- how to identify a string in a single pass
 * -------
 * Spaces are separators. If char is SPACE, this is not a string.
 * Backslash is a line break. If char is line break, this is not a string.
 * Newlines come after line breaks. If char is newline, this is not a string.
 *
 * Header paths
 * ------------
 * Header paths include .h. If string does not have .h, it is not a header path.
 *
 * Every string is a header path except the first two.
 * Instead of testing for header paths, I hardcode ignoring the first two strings.
 *******************************/
#include <stdio.h>
#include <stdbool.h>
typedef enum _state__def { STR, SEP } state;        // State is STRING or SEPARATOR
state update_state(const char c)
{
    state st = ((c == ' ') || (c == '\\') || (c == '\n')) ? SEP : STR;
    return st;
}
#define STR_OK (  (cnt>2) && (ignore==false)  )

int main(int argc, char *argv[])
{
    bool just_tag_my_libs = false;
    { // $ ./parse-headers.exe M        <---- just tag my libs
        if(argc>1) if(argv[1][0]=='M') just_tag_my_libs = true;
        printf("Tagging: %s\n",just_tag_my_libs?"my header libs only":"all libs");
    }

    FILE *f = fopen("headers-M.txt", "r");          // Read the gcc -M output
    FILE *o = fopen("headers.txt", "w");            // Write a list of headers for ctags

    { // Parse headers.txt for header paths
        state st = SEP;                             // Initial state: outside of a STRING
        int cnt = 0;                                // Initial STRING count: 0
        int c;                                      // Parse file one character at a time
        bool ignore = false;                        // State: ignore this string or not
        while( (c=fgetc(f)) != EOF )
        {
            switch(st)
            {
                case SEP:                           // Outside a STRING
                    st = update_state(c);           // Update state
                    switch(st)                      // Do action based on new state
                    {
                        case STR:                   // Start of a new STRING
                            if(just_tag_my_libs)    // Ignore paths that start with C
                            {
                                if(c=='C') ignore = true;
                                else ignore = false;
                            }
                            cnt++;                  // Increment STRING counter
                            if(STR_OK) putc(c, o);  // Print all strings after first two: "blah.o: blah.c"
                            break;
                        case SEP:                   // Still outside a STRING
                            break;                  // Do nothing
                    }
                    break;
                case STR:                           // Inside a STRING
                    st = update_state(c);           // Update state
                    switch(st)                      // Do action based on new state
                    {
                        case STR:                   // Still inside a STRING
                            if(STR_OK) putc(c, o);  // Still printing this STRING
                            break;
                        case SEP:                   // STRING is finished
                            if(STR_OK) putc('\n', o); // Add a newline after all the strings I keep
                            break;
                        default: printf("Unexpected state: %d", st); fclose(f); return 42;
                    }
                    break;
                default: printf("Unexpected state: %d", st); fclose(f); return 42;
            }
        }
    }
    fclose(o);
    fclose(f);
    return 0;
}


