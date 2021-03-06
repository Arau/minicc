
#include <iostream>
#include <fstream>
using namespace std;

#include "parser.hh"
#include "test.hh"
#include "astpr.hh"
#include "prettypr.hh"
#include "type_checker.hh"
#include "flowcontrol.hh"
#include "stepper.hh"
#include "interpreter.hh"
#include "translator.hh"
#include "walker.hh"

int main(int argc, char *argv[]) {
   string filename, todo = "eval", lang = "";
   if (argc > 1) {
      string argv1 = argv[1];
      if (argv1.substr(0, 7) == "--test-") {
         if (argc < 3) {
            cerr << argv1.substr(2) << ": missing filename" << endl;
            return 1;
         }
         filename = argv[2];
         test(argv1.substr(7), filename);
         return 0;
      } else if (argv1 == "--ast") {
         if (argc >= 3) {
            filename = argv[2];
         }
         todo = "ast";
      } else if (argv1 == "--pprint") {
         if (argc >= 3) {
            filename = argv[2];
         }
         todo = "prettyprint";
      } else if (argv1 == "--step") {
         if (argc >= 3) {
            filename = argv[2];
         }
         todo = "step";
      } else if (argv1 == "--semantic") {
        if (argc >= 3) {
            filename = argv[2];
        }
        todo = "semantic";
      } else if (argv1 == "--flowcontrol") {
        if (argc >= 3) {
            filename = argv[2];
        }
        todo = "flowcontrol";
      } else {
         filename = argv[1];
      }
   } else {
      cerr << _T("You should specify a filename") << endl;
      exit(1);
   }

   Translator::translator.set_language("en");

   ifstream codefile(filename.c_str());
   Parser P(&codefile);
   AstNode *program = P.parse();

   vector<Error*> ve;
   collect_errors(program, ve);
   for (Error *e : ve) {
      cerr << _T("Compilation Error") << ": " << e->msg << endl;
   }

   try {
      if (todo != "step") {
         AstVisitor *v;
         if (todo == "ast") {
            v = new AstPrinter(&cout);
         } else if (todo == "prettyprint") {
            v = new PrettyPrinter(&cout);
         } else if (todo == "semantic") {
            v = new TypeChecker(&cout);
         } else if (todo == "flowcontrol") {
            v = new FlowControl(&cout);
         } else {
            v = new Interpreter(&cin, &cout);
         }
         program->accept(v);
         collect_errors(program, ve);
         for (Error *e : ve) {
            cerr << e->msg << endl;
         }
         return (ve.empty() ? 0 : 1);
      } else {
         Stepper S;
         program->accept(&S);
         while (!S.finished()) {
            cout << S.span() << ": " << P.input().substr(S.span()) << endl;
            cout << S.status() << endl;
            // cout << "OUTPUT: \"" << S.output() << "\"" << endl;
            cout << endl;
            if (!S.step()) {
               throw S.error();
            }
         }
      }
   }
   catch (EvalError* e) {
      cerr << _T("Execution Error") << ": " << e->msg << endl;
      return 1;
   }
}
