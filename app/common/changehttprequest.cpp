#include "changehttprequest.h"

// ----------------------------------------------------------------------------
// ChangeHttpRequestAddLine
// ----------------------------------------------------------------------------
void ChangeHttpRequestAddLine::change(VHttpRequest& request)
{
  request.requestLine.method = "\r\n" + request.requestLine.method;
}

// ----------------------------------------------------------------------------
// ChangeHttpRequestAddSpace
// ----------------------------------------------------------------------------
void ChangeHttpRequestAddSpace::change(VHttpRequest& request)
{
  request.requestLine.method = " " + request.requestLine.method;
}

// ----------------------------------------------------------------------------
// ChangeHttpRequestDummyHost
// ----------------------------------------------------------------------------
void ChangeHttpRequestDummyHost::change(VHttpRequest& request)
{
  request.header.setValue("Host", "www.google.com");
}


// ----------------------------------------------------------------------------
// ChangeHttpRequestSslAbsPath
// ----------------------------------------------------------------------------
void ChangeHttpRequestSslAbsPath::change(VHttpRequest& request)
{
  request.requestLine.path =
    "https://" + request.header.value("Host") + request.requestLine.path;
}
