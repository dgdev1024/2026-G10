# Steps: Implementing the G10 Assembler Preprocessor

Below is a list of the top-level steps to be involved in implementing the
preprocessor and preprocessing language used by the G10 Assembler Tool (`g10asm`),
conformant to the G10 Preprocessor Language Specification (`./docs/g10pp.spec.md`).
Each step will be expanded into its own document in the `./notes` directory, and
is to contain detailed step-by-step instructions for completing the task, complete
with breakdowns, explainations, and code examples where applicable.

## Instructions

Upon a prompt to generate the next step in this document, the AI is to follow
these instructions:

1. Generate the next step in the implementation process in the "Steps" section
   below. If the step's name and details are not present in the prompt, identify
   the next logical step to be implemented according to the G10 Preprocessor
   Language Specification.
2. Create a new markdown file in the `./notes` directory, named `pp.stepN.md`,
   where `N` is the step number being generated (e.g. `pp.step1.md` for step 1).
3. Populate the new markdown file with detailed step-by-step instructions for
   completing the task, complete with breakdowns, explainations, and code
   examples where applicable. Perform this step incrementally, ensuring that
   the step is fully fleshed out without encountering any length limit issues.
4. Update this document to include a link to the newly created step document
   under the "Steps" section below.
5. Implement the step in the `g10asm` codebase, ensuring that the implementation
   conforms to the G10 Preprocessor Language Specification. Perform this step
   incrementally, ensuring that the implementation is complete and correct
   without encountering any length limit issues. Abide by the G10 Code Style
   Guide during implementation.
6. If applicable, add example G10 assembly source files and test cases to the
   `./notes/pp.tests/stepN` directory (again, `N` is the step number being
   generated; create folder if necessary) to demonstrate and verify that the
   implementation works and conforms to the specification. At the end of each
   test case file, include comments explaining what the test case is verifying,
   and the expected output string after preprocessing. Comments should be in
   the following format:
   ```asm
   ;----------------------------------------------------------------------------
   ;
   ; TEST CASE: <brief description of what the test case is verifying>
   ;
   ; <more detailed explanation of the test case>
   ;
   ; EXPECTED OUTPUT:
   ; <expected output string after preprocessing; will likely be multiple lines>
   ;
   ;----------------------------------------------------------------------------
   ```
7. If tests were generated, run the `g10asm` tool on the test cases to verify
   that the implementation is correct and conforms to the specification. Refine
   the code you generated this step to ensure correctness; try not to modify the
   tests themselves in so doing. Use the following commands to run each test file:
   - `g10asm --preprocess -s <test-file>` to display the preprocessed output
     string.
   - `g10asm --preprocess --lex -s <test-file>` to display the postpreprocessing
     lexer output.
8. If you should deem it necessary, run test cases from previous steps to ensure
   that no regressions have been introduced, that those tests still work as
   intended, conforming to the specification. As before, try not to modify the
   tests themselves in so doing, unless they may benefit from newly introduced
   functionality.

## References

- [G10 Assembler Tool (`g10asm`) Project](../projects/g10asm/)
- [G10 Code Style Guide](../docs/g10.code_style.md)
- [G10 Preprocessor Language Specification](../docs/g10pp.spec.md)
- [G10 CPU Specification](../docs/g10cpu.spec.md)
- [G10 CPU Header File](../projects/g10/cpu.hpp)
- [G10 Project Build Script](../scripts/build.sh)
- [G10 Project Clean Script](../scripts/clean.sh)

## Steps

1. [Step 1: Basic Token-to-String Output and Line Continuation Handling](./pp.step1.md)
2. [Step 2: Text-Substitution Macros (`.define` and `.undef`/`.purge`)](./pp.step2.md)
3. [Step 3: Braced Expression Interpolation](./pp.step3.md)
4. [Step 4: Extended Braced Expression Interpolation (Identifiers and Strings)](./pp.step4.md)

