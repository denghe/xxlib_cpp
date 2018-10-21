#pragma warning disable 0169, 0414
using TemplateLibrary;

// login server send to client
namespace Login_Client
{
    class Auth_Success
    {
        Server.Types type;
        string ip;
        int port;
        string token;
    }
}
