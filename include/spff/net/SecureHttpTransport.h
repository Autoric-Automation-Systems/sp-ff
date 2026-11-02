#pragma once

#include "spff/net/ITransport.h"

namespace spff
{

// POST HTTPS com validação de certificado, timeouts e chave compartilhada.
class SecureHttpTransport : public ITransport
{
public:
  int post(const char *url, const char *body, char *response, size_t responseSize) override;
};

} // namespace spff
