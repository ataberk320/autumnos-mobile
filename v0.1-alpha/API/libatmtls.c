#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>

SSL_CTX *tls = NULL;
// we have an path already.
int AutumnAPI_TLSSec() {
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	const SSL_METHOD *method = TLS_client_method();
	tls = SSL_CTX_new(method);
	if (!tls) {
		fprintf(stderr, "Could not create SSL context!\n");
        	ERR_print_errors_fp(stderr);
        	return -1;
    	}

	if (SSL_CTX_load_verify_locations(tls, "/etc/ssl/certs/ca-certificates.crt", NULL) != 1) {
		fprintf(stderr, "Could not load CA certificate!\n");
		SSL_CTX_free(tls);
		tls = NULL;
		return -1;
	}
//i should be careful :)
	SSL_CTX_set_verify(tls, SSL_VERIFY_PEER, NULL);
    	SSL_CTX_set_min_proto_version(tls, TLS1_2_VERSION); //tls 1.2+!

    	return 0;
}
		
void AutumnAPI_HostnameConf(SSL *ssl, const char *hname) {
	if (SSL_set_tlsext_host_name(ssl, hname) != 1) {
		return;
	}

	X509_VERIFY_PARAM *param = SSL_get0_param(ssl); //controlling matching with hostname
	if (!param) {
		return;
	}

	if (X509_VERIFY_PARAM_set1_host(param, hname, 0) != 1) { //my fault :)
        	return;
    	}

    	return 0;
}
	
