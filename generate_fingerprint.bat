REM download: https://www.geotrust.com/resources/root_certificates/certificates/Equifax_Secure_Certificate_Authority.pem
REM download: https://indy.fulgan.com/SSL/openssl-1.0.2l-i386-win32.zip

echo | openssl s_client -CAfile Equifax_Secure_Certificate_Authority.pem -connect script.google.com:443 | openssl x509 -fingerprint -noout 
pause