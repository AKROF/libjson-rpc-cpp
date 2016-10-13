#include "python_client_stub_generator.h"
#include <algorithm>

#define TEMPLATE_PYTHON_CLIENT_SIGCLASS "class <stubname>(client.Client):"

#define TEMPLATE_PYTHON_CLIENT_SIGCONSTRUCTOR "def __init__(self, connector, version='2.0'):\n        super(<stubname>, self).__init__(connector, version)"

#define TEMPLATE_PYTHON_CLIENT_SIGMETHOD "def <methodname>(self<parameters>):"

#define TEMPLATE_NAMED_ASSIGNMENT "parameters[\'<paramname>\'] = <paramname>"
#define TEMPLATE_POSITION_ASSIGNMENT "parameters.append(<paramname>)"

#define TEMPLATE_METHODCALL "result = self.callMethod(\'<name>\', parameters)"
#define TEMPLATE_NOTIFICATIONCALL "self.callNotification(\'<name>\', parameters)"

using namespace std;
using namespace jsonrpc;


PythonClientStubGenerator::PythonClientStubGenerator(const string &stubname, std::vector<Procedure> &procedures, std::ostream& outputstream) :
    StubGenerator(stubname, procedures, outputstream)
{
}

PythonClientStubGenerator::PythonClientStubGenerator(const string &stubname, std::vector<Procedure> &procedures, const string filename) :
    StubGenerator(stubname, procedures, filename)
{
}

void PythonClientStubGenerator::generateStub()
{
    this->writeLine("#");
    this->writeLine("# This file is generated by jsonrpcstub, DO NOT CHANGE IT MANUALLY!");
    this->writeLine("#");
    this->writeNewLine();
    this->writeLine("import client");
    this->writeNewLine();

    this->writeLine(replaceAll(TEMPLATE_PYTHON_CLIENT_SIGCLASS, "<stubname>", this->stubname));
    this->increaseIndentation();

    this->writeLine(replaceAll(TEMPLATE_PYTHON_CLIENT_SIGCONSTRUCTOR, "<stubname>", this->stubname));
    this->writeNewLine();

    for (unsigned int i=0; i < procedures.size(); i++)
    {
        this->generateMethod(procedures[i]);
    }

    this->decreaseIndentation();
    this->writeNewLine();
}

void PythonClientStubGenerator::generateMethod(Procedure &proc)
{
    string procsignature = TEMPLATE_PYTHON_CLIENT_SIGMETHOD;
    replaceAll2(procsignature, "<methodname>", proc.GetProcedureName());

    // generate parameters string
    string params = generateParameterDeclarationList(proc);
    replaceAll2(procsignature, "<parameters>", params);

    this->writeLine(procsignature);
    this->increaseIndentation();

    generateAssignments(proc);
    this->writeNewLine();
    generateProcCall(proc);
    this->writeNewLine();

    this->decreaseIndentation();
}

string PythonClientStubGenerator::generateParameterDeclarationList(Procedure &proc)
{
    stringstream param_string;
    parameterNameList_t list = proc.GetParameters();

    for (parameterNameList_t::iterator it = list.begin(); it != list.end(); ++it)
    {
        param_string << ", ";
        param_string << it->first;
    }

    return param_string.str();
}

void PythonClientStubGenerator::generateAssignments(Procedure &proc)
{
    string assignment;
    parameterNameList_t list = proc.GetParameters();
    if(list.size() > 0)
    {
        parameterDeclaration_t declType = proc.GetParameterDeclarationType();
        if (proc.GetParameterDeclarationType() == PARAMS_BY_NAME)
        {
            this->writeLine("parameters = {}");
        }
        else if(proc.GetParameterDeclarationType() == PARAMS_BY_POSITION)
        {
            this->writeLine("parameters = []");
        }

        for (parameterNameList_t::iterator it = list.begin(); it != list.end(); ++it)
        {

            if(declType == PARAMS_BY_NAME)
            {
                assignment = TEMPLATE_NAMED_ASSIGNMENT;
            }
            else
            {
                assignment = TEMPLATE_POSITION_ASSIGNMENT;
            }
            replaceAll2(assignment, "<paramname>", it->first);
            this->writeLine(assignment);
        }
    }
    else
    {
        this->writeLine("parameters = None");
    }

}

void PythonClientStubGenerator::generateProcCall(Procedure &proc)
{
    string call;
    if (proc.GetProcedureType() == RPC_METHOD)
    {
        call = TEMPLATE_METHODCALL;
        this->writeLine(replaceAll(call, "<name>", proc.GetProcedureName()));
        this->writeLine("return result");
    }
    else
    {
        call = TEMPLATE_NOTIFICATIONCALL;
        replaceAll2(call, "<name>", proc.GetProcedureName());
        this->writeLine(call);
    }

}

string PythonClientStubGenerator::class2Filename(const string &classname)
{
    string result = classname;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result + ".py";
}
