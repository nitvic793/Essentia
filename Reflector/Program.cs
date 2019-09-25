using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using CppAst;

namespace Parser
{
    class FieldType
    {
        string TypeName;
    }

    class ClassType
    {
        public Dictionary<string, string> Fields = new Dictionary<string, string>();
    }

    class TranslationUnit
    {
        public Dictionary<string, ClassType> ReflectedClasses = new Dictionary<string, ClassType>();
    }

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
            var directory = @"E:\Projects\Sandbox\Sandbox"; Directory.GetCurrentDirectory();
            var files = Directory.EnumerateFiles(directory, "*.h", SearchOption.AllDirectories).ToList();
            var options = new CppAst.CppParserOptions
            {
                ParseMacros = true,
                Defines = { "Parser" }
            };

            Dictionary<string, TranslationUnit> translationUnits = new Dictionary<string, TranslationUnit>();



            bool outRequired = false;
            foreach (var file in files)
            {
                Console.WriteLine("Running Reflector... " + file);

                var ast = CppParser.ParseFile(file, options);
                foreach (var Class in ast.Classes)
                {
                    var tUnit = Class.Span.Start.File;
                    tUnit = tUnit.Replace("/", @"\");
                    if (NeedsReflection(Class))
                    {
                        if (!translationUnits.ContainsKey(tUnit))
                        {
                            translationUnits.Add(tUnit, new TranslationUnit());
                        }
                        else
                        {
                            continue;
                        }

                        outRequired = true;
                        if (!translationUnits[tUnit].ReflectedClasses.ContainsKey(Class.Name))
                        {
                            translationUnits[tUnit].ReflectedClasses.Add(Class.Name, new ClassType { Fields = new Dictionary<string, string>() });
                        }
                        else continue;

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
                                    if (!translationUnits[tUnit].ReflectedClasses[Class.Name].Fields.ContainsKey(type.Name))
                                        translationUnits[tUnit].ReflectedClasses[Class.Name].Fields.Add(type.Name, cls.Name);
                                }
                                else if (type.Type.TypeKind == CppTypeKind.Primitive)
                                {
                                    var cls = (CppPrimitiveType)type.Type;
                                    Console.WriteLine(cls.Kind.ToString());
                                    if (!translationUnits[tUnit].ReflectedClasses[Class.Name].Fields.ContainsKey(type.Name))
                                        translationUnits[tUnit].ReflectedClasses[Class.Name].Fields.Add(type.Name, cls.Kind.ToString());
                                }
                            }
                        }
                    }
                }
            }


            if (outRequired)
            {
                foreach(var translationUnit in translationUnits)
                {
                    var path = Path.GetDirectoryName(translationUnit.Key);
                }
            }

        }
    }
}
