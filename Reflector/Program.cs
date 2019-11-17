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
        public string TypeName;
        public int Size;
        public int Offset;
    }

    class ClassType
    {
        public Dictionary<string, FieldType> Fields = new Dictionary<string, FieldType>();
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

        static Dictionary<string, TranslationUnit> GetTranslationUnits(List<string> files)
        {
            var options = new CppAst.CppParserOptions
            {
                ParseMacros = true,
                Defines = { "Parser" },
                ParseAsCpp = true
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
                            translationUnits[tUnit].ReflectedClasses.Add(Class.Name, new ClassType { Fields = new Dictionary<string, FieldType>() });
                        }
                        else continue;

                        int offset = 0;
                        foreach (var type in Class.Fields)
                        {
                            if (NeedsReflection(type))
                            {
                                var fieldType = new FieldType();
                                fieldType.Offset = offset;
                                //Console.WriteLine(type.Name);
                                if (type.Type.TypeKind == CppTypeKind.StructOrClass)
                                {
                                    var cls = (CppClass)type.Type;
                                    //Console.WriteLine(cls.Name);
                                    if (!translationUnits[tUnit].ReflectedClasses[Class.Name].Fields.ContainsKey(type.Name))
                                    {
                                        fieldType.TypeName = cls.Name;
                                        fieldType.Size = cls.SizeOf;
                                        translationUnits[tUnit].ReflectedClasses[Class.Name].Fields.Add(type.Name, fieldType);
                                    }
                                }
                                else if (type.Type.TypeKind == CppTypeKind.Primitive)
                                {
                                    // Console.WriteLine(cls.Kind.ToString());
                                    if (!translationUnits[tUnit].ReflectedClasses[Class.Name].Fields.ContainsKey(type.Name))
                                    {
                                        ;
                                        fieldType.TypeName = type.Type.GetDisplayName();
                                        fieldType.Size = type.Type.SizeOf;
                                        translationUnits[tUnit].ReflectedClasses[Class.Name].Fields.Add(type.Name, fieldType);
                                    }
                                }
                            }

                            offset += type.Type.SizeOf;
                        }
                    }
                }
            }

            return translationUnits;
        }

        static void Main(string[] args)
        {
            var directory = @"E:\Projects\Sandbox\Sandbox"; Directory.GetCurrentDirectory();
            var files = Directory.EnumerateFiles(directory, "*.h", SearchOption.AllDirectories).ToList();


            Dictionary<string, TranslationUnit> translationUnits = GetTranslationUnits(files);


            var outHeader = "Generated.h";
            var outCpp = "Generated.cpp";
            List<string> includes = new List<string>();
            var includeLine = "#include <{0}>";

            var startFunction = "void InitReflection() {";
            var endFunction = "}";

            if (true)
            {
                foreach (var translationUnit in translationUnits)
                {
                    var path = Path.GetDirectoryName(translationUnit.Key);
                    var include = Path.GetFileName(translationUnit.Key);
                    includes.Add(include);
                }

                foreach (var translationUnit in translationUnits)
                {
                    foreach (var cls in translationUnit.Value.ReflectedClasses)
                    {
                        Console.WriteLine(cls.Key);
                        foreach (var field in cls.Value.Fields)
                        {
                            Console.WriteLine(field.Key + " " + field.Value.TypeName);
                        }
                    }
                }
            }

        }
    }
}
