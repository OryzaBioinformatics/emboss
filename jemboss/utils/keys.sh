#!/bin/sh

# Tim carver 
# keys.sh
#
# Creates RSA keystores
#
# Usage: keys.sh directory_to_put_keys hostname key_password
#
# 13 March 02



ssl_create_keystore()
{

  KEYSTORE=$1
  ALIAS=$2
  HOST=$3
  JEMBOSS_RES=$4

  keytool -genkey -alias $ALIAS -dname "CN=$HOST, \
      OU=Jemboss, O=HGMP-RC, L=CAMBRIDGE, S=CAMBRIDGE, C=UK" -keyalg RSA \
      -keypass $PASSWD -storepass $PASSWD -keystore $JEMBOSS_RES/$KEYSTORE.keystore  

  keytool -export -alias $ALIAS -storepass $PASSWD -file $JEMBOSS_RES/$KEYSTORE.cer \
      -keystore $JEMBOSS_RES/$KEYSTORE.keystore

}

ssl_import()
{
  FILE=$1
  KEYSTORE=$2
  JEMBOSS_RES=$3

  echo "**********IMPORTING"
  keytool -import -v -trustcacerts -alias tomcat -file $JEMBOSS_RES/$FILE -keystore \
            $JEMBOSS_RES/$KEYSTORE -keypass $PASSWD -storepass $PASSWD -noprompt

}

JEMBOSS_RES=$1
HOST=$2
PASSWD=$3

#
# create the keystores & export the certificates
#
ssl_create_keystore "server" "tomcat-sv" $HOST $JEMBOSS_RES $PASSWD
ssl_create_keystore "client" "tomcat-cl" $HOST $JEMBOSS_RES $PASSWD

#
# import certificates into keystores - so server trusts client...
#
ssl_import server.cer client.keystore $JEMBOSS_RES $PASSWD
ssl_import client.cer server.keystore $JEMBOSS_RES $PASSWD

exit 0;


