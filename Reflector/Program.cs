using CppAst;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Reflector
{
    class Program
    {
        static bool NeedsReflection(CppClass Class)
        {
            return Class.Comment != null && Class.Comment.Children[0].ToString().Contains("@Reflect");
        }

        static bool NeedsReflection(CppField Field)
        {
            return Field.Comment != null && Field.Comment.Children[0].ToString().Contains("@Reflect");
        }

        static void Main(string[] args)
        {
            var options = new CppAst.CppParserOptions
            {
                ParseMacros = true,
                Defines = { "Parser" }
            };

            var ast = CppParser.ParseFile("Test.h", options);
            foreach (var Class in ast.Classes)
            {
                if (NeedsReflection(Class))
                {
                    Console.WriteLine(Class.Name);
                    foreach (var type in Class.Fields)
                    {
                        if (NeedsReflection(type))
                        {
                            Console.WriteLine(type.Name);
                            if (type.Type.TypeKind == CppTypeKind.StructOrClass)
                            {
                                var cls = (CppClass)type.Type;
                                Console.WriteLine(cls.Name);
                            }
                            else if (type.Type.TypeKind == CppTypeKind.Primitive)
                            {
                                var cls = (CppPrimitiveType)type.Type;
                                Console.WriteLine(cls.Kind.ToString());
                            }
                        }
                    }
                }
            }
        }
    }
}
