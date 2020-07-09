#!/bin/sh
# 
# install-jemboss-server.sh
# 1/5/02



tomcat_classpath_notes()
{

 TOMCAT_ROOT=$1
 JEMBOSS_ROOT=$2

 echo
 echo "Add the classpath to jemboss to Tomcat. This means editing"
 echo "$TOMCAT_ROOT/bin/catalina.sh to add the line: "
 echo "CLASSPATH=\"$CLASSPATH\":"$JEMBOSS_ROOT:$JEMBOSS_ROOT/lib/soap.jar
 echo

}


ssl_print_notes()
{

 KEYSTOREFILE=$1
 TOMCAT_ROOT=$2
 PORT=$3
 
 echo 
 echo
 if [ -f $JAVA_HOME/jre/lib/security/java.security ]; then
   echo "A) EDIT "$JAVA_HOME/jre/lib/security/java.security
 else
   echo "A) EDIT the java.security file"
 fi

 echo "   adding/changing the provider line (usually provider 2 or 3):"
 echo "   security.provider.2=com.sun.net.ssl.internal.ssl.Provider"
 echo
 echo "B) COPY & PASTE THE FOLLOWING INTO (changing port number if required) "$TOMCAT_ROOT/conf/server.xml
 echo
 echo '   <!-- Define an SSL HTTP/1.1 Connector on port '$PORT' -->'
 echo '   <Connector className="org.apache.catalina.connector.http.HttpConnector"'
 echo '           port="'$PORT'" minProcessors="5" maxProcessors="75"'
 echo '           enableLookups="true"'
 echo '           acceptCount="10" debug="0" scheme="https" secure="true">'
 echo '   <Factory className="org.apache.catalina.net.SSLServerSocketFactory"'
 echo '           keystoreFile="'$KEYSTOREFILE'" keystorePass="'$PASSWD'"'
 echo '           clientAuth="false" protocol="TLS"/>'
 echo '   </Connector>'  
 echo 

}

ssl_create_keystore()
{

  HOST=$1
  JEMBOSS_RES=$2
  KEYSTORE=$3
  ALIAS=$4
  PASSWD=$5

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
  PASSWD=$3

  keytool -import -v -trustcacerts -alias tomcat -file $FILE -keystore \
            $KEYSTORE -keypass $PASSWD -storepass $PASSWD -noprompt

}


make_jemboss_properties()
{

  EMBOSS_INSTALL=$1
  URL=$2
  AUTH=$3
  SSL=$4
  PORT=$5

  if [ $SSL = "y" ]; then
     URL=https://$URL:$PORT
  else
     URL=http://$URL:$PORT
  fi
  

  JEMBOSS_PROPERTIES=$EMBOSS_INSTALL/share/EMBOSS/jemboss/resources/jemboss.properties

  mv $JEMBOSS_PROPERTIES $JEMBOSS_PROPERTIES.orig

  touch $JEMBOSS_PROPERTIES

  if [ $AUTH = "y" ]; then
    echo "user.auth=true" > $JEMBOSS_PROPERTIES
  else
    echo "user.auth=false" > $JEMBOSS_PROPERTIES
  fi
  echo "jemboss.server=true" >> $JEMBOSS_PROPERTIES
  echo "server.public=$URL/soap/servlet/rpcrouter" \
                                      >> $JEMBOSS_PROPERTIES

  echo "server.private=$URL/soap/servlet/rpcrouter" \
                                       >> $JEMBOSS_PROPERTIES

  if [ $AUTH = "y" ]; then
   echo "service.public=JembossAuthServer" >> $JEMBOSS_PROPERTIES
   echo "service.private=JembossAuthServer" >> $JEMBOSS_PROPERTIES
  else
   echo "service.public=JembossServer" >> $JEMBOSS_PROPERTIES
   echo "service.private=JembossServer" >> $JEMBOSS_PROPERTIES
  fi

  echo "plplot=$EMBOSS_INSTALL/share/EMBOSS/" >> $JEMBOSS_PROPERTIES
  echo "embossData=$EMBOSS_INSTALL/share/EMBOSS/data/" >> $JEMBOSS_PROPERTIES
  echo "embossBin=$EMBOSS_INSTALL/bin/" >> $JEMBOSS_PROPERTIES
  echo "embossPath=/usr/bin/:/bin:/packages/clustal/:/packages/primer3/bin:" \
                                                     >> $JEMBOSS_PROPERTIES
  echo "acdDirToParse=$EMBOSS_INSTALL/share/EMBOSS/acd/" >> $JEMBOSS_PROPERTIES

  cp $JEMBOSS_PROPERTIES $JEMBOSS_PROPERTIES.bak

  echo
  echo "Changed $EMBOSS_INSTALL/share/EMBOSS/jemboss/resources/jemboss.properties"
  echo "to reflect this installation (original in jemboss.properties.orig)"
  echo

}


deploy_auth_services()
{

  JEMBOSS_LIB=$1
  CLASSPATH=$JEMBOSS_LIB/soap.jar:$JEMBOSS_LIB/activation.jar:$JEMBOSS_LIB/xerces.jar:$JEMBOSS_LIB/mail.jar
  CLASSPATH=${CLASSPATH}:$JEMBOSS_LIB/jnet.jar:$JEMBOSS_LIB/jsse.jar:$JEMBOSS_LIB/jcert.jar

  SERVICE=$2
  URL=$3
  JAVAHOME=$4
  OPT_PROP1=$5
  OPT_PROP2=$6

  echo
  echo "Deploying $SERVICE "
  echo "$JAVAHOME/bin/java -classpath $CLASSPATH $OPT_PROP1 $OPT_PROP2 org.apache.soap.server.ServiceManagerClient  $URL/soap/servlet/rpcrouter deploy $SERVICE"
  echo

  $JAVAHOME/bin/java -classpath $CLASSPATH $OPT_PROP1 $OPT_PROP2 \
        org.apache.soap.server.ServiceManagerClient \
        $URL/soap/servlet/rpcrouter deploy $SERVICE

}


output_auth_xml()
{
  AUTH=$3
  if [ $AUTH = "y" ]; then
    JEM_CLASS="org.emboss.jemboss.server.JembossAuthServer"
    FIL_CLASS="org.emboss.jemboss.server.JembossFileAuthServer"
    ID="JembossAuthServer"
  else
    JEM_CLASS="org.emboss.jemboss.server.JembossServer"
    FIL_CLASS="org.emboss.jemboss.server.JembossFileServer"
    ID="JembossServer"
  fi

  XML_FILE=$1
  echo '<isd:service xmlns:isd="http://xml.apache.org/xml-soap/deployment"' > $XML_FILE
  echo " id=\"$ID\">" >> $XML_FILE
  echo ' <isd:provider type="java"' >> $XML_FILE
  echo '  scope="Request"' >> $XML_FILE
  echo '  methods="name version abouturl show_acd getWossname' >> $XML_FILE
  echo '  show_help show_db run_prog show_saved_results' >> $XML_FILE
  echo '  delete_saved_results list_saved_results call_ajax' >> $XML_FILE
  echo '  update_result_status">' >> $XML_FILE
  echo '  <isd:java class="'$JEM_CLASS'"' >> $XML_FILE
  echo '  static="false"/>' >> $XML_FILE
  echo ' </isd:provider>' >>$XML_FILE
  echo ' <isd:faultListener>' >>$XML_FILE
  echo '   org.apache.soap.server.DOMFaultListener' >> $XML_FILE
  echo ' </isd:faultListener>' >> $XML_FILE
  echo '</isd:service>' >> $XML_FILE

  XML_FILE1=$2
  echo '<isd:service xmlns:isd="http://xml.apache.org/xml-soap/deployment"' > $XML_FILE1
  echo ' id="EmbreoFile">' >> $XML_FILE1
  echo ' <isd:provider type="java"' >> $XML_FILE1
  echo '  scope="Request"' >> $XML_FILE1
  echo '  methods="directory_shortls embreo_roots get_file put_file">' >> $XML_FILE1
  echo '  <isd:java class="'$FIL_CLASS'"' >> $XML_FILE1
  echo '  static="false"/>' >> $XML_FILE1
  echo ' </isd:provider>' >>$XML_FILE1
  echo ' <isd:faultListener>' >>$XML_FILE1
  echo '   org.apache.soap.server.DOMFaultListener' >> $XML_FILE1
  echo ' </isd:faultListener>' >> $XML_FILE1
  echo '</isd:service>' >> $XML_FILE1
  
}


echo
echo "--------------------------------------------------------------"
echo "         EMBOSS and Jemboss Server installation script"
echo "--------------------------------------------------------------"
echo
echo "*** This script needs to be run with permissions to be able"
echo "*** to install EMBOSS in the required directories. This may"
echo "*** be best done as root or as a tomcat user."
echo
echo "Before running this script you should download the latest:"
echo
echo "(1) EMBOSS release (contains Jemboss) ftp://ftp.hgmp.mrc.ac.uk/pub/EMBOSS/"
echo "(2) Tomcat release http://jakarta.apache.org/site/binindex.html"
echo "(3) SOAP release   http://xml.apache.org/dist/soap/"  
echo
echo "Have all these been downloaded (y/n)? "
read DOWNLOADED

if [ "$DOWNLOADED" != "y" ]; then
  if [ "$DOWNLOADED" != "Y" ]; then
    exit 1
  fi
fi


echo 
echo "Select your platform from 1-6:"
echo "(1)  linux"
echo "(2)  aix"
echo "(3)  irix"
echo "(4)  hp-ux"
echo "(5)  solaris"
echo "(6)  macosX"
read PLAT

AIX="n"
MACOSX="n"
if [ "$PLAT" = "1" ]; then
  PLATFORM="linux"
elif [ "$PLAT" = "2" ]; then
  PLATFORM="aix"
  AIX="y"
elif [ "$PLAT" = "3" ]; then
  PLATFORM="irix"
elif [ "$PLAT" = "4" ]; then
  PLATFORM="hpux"
elif [ "$PLAT" = "5" ]; then
  PLATFORM="solaris"
elif [ "$PLAT" = "6" ]; then
  PLATFORM="macos"
  MACOSX="y"
else
  echo "Platform not selected from 1-6."
  exit 1
fi

#
# localhost name
#
echo
echo "Enter IP of server machine [localhost]:"
read LOCALHOST

if [ "$LOCALHOST" = "" ]; then
  LOCALHOST=localhost
fi


#
# SSL
#
echo
echo "Enter if you want the Jemboss (SOAP) server to use"
echo "data encryption with Secure Socket Layer (y,n) [y]?"
read SSL

if [ "$SSL" = "" ]; then
  SSL="y"
fi

JSSE_HOME=""

if [ $SSL = "y" ]; then
  PORT=8443
else
  PORT=8080
fi

#
# PORT
#
USER_PORT=""
echo
echo "Enter port number [$PORT]:"
read USER_PORT

if [ "$USER_PORT" = "" ]; then
  if [ $SSL = "y" ]; then
    PORT=8443
  else
    PORT=8080
  fi
else
  PORT=$USER_PORT
fi

#
# JAVA_HOME
#

JAVA_HOME=0

while [ ! -f "$JAVA_HOME/bin/javac" ]
do
  echo "Enter java (1.3 or above) location (/usr/java/jdk1.3.1/): "
  read JAVA_HOME
done

#
# add java bin to path
#
PATH=$PATH:$JAVA_HOME/bin/ ; export PATH

#
#
# JNI location for linux/solaris/AIX/SGI/HP-UX
#
#  
#
JAVA_INCLUDE=$JAVA_HOME/include/
JAVA_INCLUDE_OS=$JAVA_INCLUDE
if [ -d $JAVA_INCLUDE/linux ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}/linux
elif [ -d $JAVA_INCLUDE/solaris ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}/solaris
elif [ -d $JAVA_INCLUDE/irix ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}/irix
elif [ -d $JAVA_INCLUDE/hp-ux ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}/hp-ux
elif [ -d $JAVA_INCLUDE ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}
else
  echo "Enter java include/Header directory location (containing jni.h)? "
  read JAVA_INCLUDE
  JAVA_INCLUDE_OS=$JAVA_INCLUDE
  if [ -d $JAVA_INCLUDE/linux ]; then
    JAVA_INCLUDE_OS=${JAVA_INCLUDE}/linux
  elif [ -d $JAVA_INCLUDE/solaris ]; then
    JAVA_INCLUDE_OS=${JAVA_INCLUDE}/solaris
  elif [ -d $JAVA_INCLUDE/irix ]; then
    JAVA_INCLUDE_OS=${JAVA_INCLUDE}/irix
  elif [ -d $JAVA_INCLUDE ]; then
    JAVA_INCLUDE_OS=${JAVA_INCLUDE}
  else
    echo "Problems finding java include libraries!"
    exit 1
  fi
fi

#
# EMBOSS_DOWNLOAD
#
EMBOSS_DOWNLOAD=0

while [ ! -d "$EMBOSS_DOWNLOAD" ]
do
  echo "Enter EMBOSS download directory (e.g. /usr/emboss/EMBOSS-2.x.x): "
  read EMBOSS_DOWNLOAD
done

echo "Enter where EMBOSS should be installed [/usr/local/emboss]: "
read EMBOSS_INSTALL

if [ "$EMBOSS_INSTALL" = "" ]; then
  EMBOSS_INSTALL=/usr/local/emboss
fi

#
# set JSSE_HOME to the EMBOSS install dir
#
JSSE_HOME=$EMBOSS_INSTALL/share/EMBOSS/jemboss
JEMBOSS_SERVER_AUTH=""
AUTH=y
echo "Enter if you want Jemboss to use unix authorisation (y/n) [y]? "
read AUTH

if [ "$AUTH" = "" ]; then
  AUTH="y"
fi


if [ "$AUTH" = "y" ]; then

  echo "Provide the UID of the account (non-priveleged) to run Tomcat,"
  echo "it has to be greater than 100 [506]:"
  read UUID

  if [ "$UUID" = "" ]; then
    UUID="506"
  fi

  if [ "$UUID" != "" ]; then
    echo "int dummy(){}" > dummy.c
    if (cc dummy.c -c -o dummy.o >/dev/null 2>&1); then
      CC="cc -DTOMCAT_UID=$UUID "; export CC
    else
      CC="gcc -DTOMCAT_UID=$UUID "; export CC
    fi
    rm -f dummy.c dummy.o 
  fi

  echo
  echo
  echo "(1) shadow      (3) PAM         (5) HP-UX shadow"
  echo "(2) no shadow   (4) AIX shadow  (6) Re-entrant shadow"
  echo "(7) Pain re-entrant"  
  echo 
  echo "Type of unix password method being used "
  echo "(select 1, 2, 3, 4, 5, 6 or 7 )[1]"

  read AUTH_TYPE
  
  if [ "$AUTH_TYPE" = "1" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=shadow"
  elif [ "$AUTH_TYPE" = "2" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=noshadow"
  elif [ "$AUTH_TYPE" = "3" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=pam"
  elif [ "$AUTH_TYPE" = "4" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=aixshadow"
  elif [ "$AUTH_TYPE" = "5" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=hpuxshadow"
  elif [ "$AUTH_TYPE" = "6" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=rshadow"
  elif [ "$AUTH_TYPE" = "7" ]; then
    JEMBOSS_SERVER_AUTH=" --with-auth=rnoshadow"
  else
    JEMBOSS_SERVER_AUTH=" --with-auth=shadow"
  fi
fi
 

#
#
# Tomcat
#
TOMCAT_ROOT=0

while [ ! -d "$TOMCAT_ROOT/webapps" ]
do
  echo "Enter Tomcat root directory (e.g. /usr/local/jakarta-tomcat-4.x.x)"
  read TOMCAT_ROOT
done

#
# SOAP
#
SOAP_ROOT=0

while [ ! -f "$SOAP_ROOT/webapps/soap.war" ]
do
  echo "Enter SOAP root directory (e.g. /usr/local/soap-2_x)"
  read SOAP_ROOT
done

cp $SOAP_ROOT/webapps/soap.war $TOMCAT_ROOT/webapps

#
#
#

USER_CONFIG=""
echo "Enter any other configuration options (e.g. --with-pngdriver=pathname"
echo "or press return to leave blank):"
read USER_CONFIG

#
#
if [ "$AIX" = "y" ]; then
  if [ "$AUTH" = "y" ]; then
    CC="xlc_r -DTOMCAT_UID=$UUID "; export CC
  else
    CC=xlc_r; export CC
  fi
fi


echo
echo "  ******** EMBOSS will be configured with this information  ******** "
echo 
echo "./configure --with-java=$JAVA_INCLUDE \\"
echo "            --with-javaos=$JAVA_INCLUDE_OS \\"
echo "            --with-thread=$PLATFORM \\"
echo "            --prefix=$EMBOSS_INSTALL $JEMBOSS_SERVER_AUTH \\"
echo "            $USER_CONFIG"
echo
sleep 3

WORK_DIR=$PWD
cd $EMBOSS_DOWNLOAD

./configure --with-java=$JAVA_INCLUDE \
            --with-javaos=$JAVA_INCLUDE_OS \
            --with-thread=$PLATFORM \
            --prefix=$EMBOSS_INSTALL $JEMBOSS_SERVER_AUTH $USER_CONFIG

make

echo
echo "  ******* EMBOSS with Jemboss will be installed in $EMBOSS_INSTALL ******* "
echo
sleep 5

make install

#
#
# Compile server code
#

#cd $EMBOSS_INSTALL/share/EMBOSS/jemboss
JEMBOSS=$EMBOSS_INSTALL/share/EMBOSS/jemboss

if [ $AUTH = "y" ]; then
  $JAVA_HOME/bin/javac -classpath $JEMBOSS:$JEMBOSS/lib/soap.jar $JEMBOSS/org/emboss/jemboss/server/JembossAuthServer.java
  $JAVA_HOME/bin/javac -classpath $JEMBOSS:$JEMBOSS/lib/soap.jar $JEMBOSS/org/emboss/jemboss/server/JembossFileAuthServer.java
else
  $JAVA_HOME/bin/javac -classpath $JEMBOSS:$JEMBOSS/lib/soap.jar $JEMBOSS/org/emboss/jemboss/server/JembossServer.java
  $JAVA_HOME/bin/javac -classpath $JEMBOSS:$JEMBOSS/lib/soap.jar $JEMBOSS/org/emboss/jemboss/server/JembossFileServer.java
fi

cd $WORK_DIR



if [ "$MACOSX" = "y" ]; then
  ln -s $EMBOSS_INSTALL/lib/libajax.dylib $EMBOSS_INSTALL/lib/libajax.jnilib
# cp $EMBOSS_INSTALL/lib/libajax.[0-9].dylib /System/Library/Frameworks/JavaVM.framework/Libraries/libajax.jnilib
fi

if [ "$PLATFORM" = "hpux" ]; then
  ln -s $EMBOSS_INSTALL/lib/libajax.sl $EMBOSS_INSTALL/lib/libajax.so
fi

#
#
# Tomcat scripts
#
#

rm -f tomstart

echo
echo "#!/bin/csh " > tomstart
echo "setenv JAVA_HOME $JAVA_HOME" >> tomstart

if [ "$SSL" = "y" ]; then
  echo "setenv JSSE_HOME $JSSE_HOME" >> tomstart
fi
 
if [ "$AIX" = "y" ]; then
 echo "setenv LIBPATH /usr/lib/threads:/usr/lib:/lib:$EMBOSS_INSTALL/lib" >> tomstart
 cp $EMBOSS_DOWNLOAD/ajax/.libs/libajax.so.0 $EMBOSS_INSTALL/lib
 ln -s $EMBOSS_INSTALL/lib/libajax.so.0 $EMBOSS_INSTALL/lib/libajax.so
else
 echo "setenv LD_LIBRARY_PATH $EMBOSS_INSTALL/lib" >> tomstart
fi

if [ "$MACOSX" = "y" ]; then
 echo "setenv DYLD_LIBRARY_PATH /usr/local/emboss/lib" >> tomstart
fi

if [ "$PLATFORM" = "hpux" ]; then
 echo "setenv SHLIB_PATH $EMBOSS_INSTALL/lib" >> tomstart
fi


if [ "$AUTH_TYPE" = "3" ]; then
  if [ -f "/lib/libpam.so" ]; then
    echo "setenv LD_PRELOAD /lib/libpam.so" >> tomstart
  elif [ -f "/usr/lib/libpam.so" ]; then
    echo "setenv LD_PRELOAD /usr/lib/libpam.so" >> tomstart
  else
    echo
    echo "WARNING: don't know what to set LD_PRELOAD to"
    echo "edit LD_PRELOAD in tomstart script!"
    echo "setenv LD_PRELOAD /usr/lib/libpam.so" >> tomstart
    echo
  fi
fi

echo 'set path=($path '"$JAVA_HOME/bin)"  >> tomstart
echo "rehash"  >> tomstart
echo "$TOMCAT_ROOT/bin/startup.sh"  >> tomstart


rm -f tomstop

echo
echo "#!/bin/csh " > tomstop
echo "setenv JAVA_HOME $JAVA_HOME" >> tomstop
echo "setenv LD_LIBRARY_PATH $EMBOSS_INSTALL/lib" >> tomstop
if [ "$AUTH_TYPE" = "3" ]; then
  echo "setenv LD_PRELOAD /lib/libpam.so" >> tomstop
fi
echo 'set path=($path '"$JAVA_HOME/bin)"  >> tomstop
echo "rehash"  >> tomstop
echo "$TOMCAT_ROOT/bin/shutdown.sh"  >> tomstop


chmod u+x tomstart
chmod u+x tomstop

#
#
# Create XML deployment descriptor files
#

#JEMBOSS=$EMBOSS_INSTALL/share/EMBOSS/jemboss
output_auth_xml JembossAuthServer.xml JembossFileAuthServer.xml $AUTH

if [ "$SSL" != "y" ]; then

  echo
  echo "Tomcat XML deployment descriptors have been created for the Jemboss Server."
  echo "Would you like an automatic deployment of these to be tried (y/n)?"
  read DEPLOYSERVICE

  if [ "$DEPLOYSERVICE" = "y" ]; then
    ./tomstart
    echo
    echo "Please wait, starting tomcat......."
    sleep 25
    deploy_auth_services $JEMBOSS/lib JembossAuthServer.xml http://$LOCALHOST:$PORT/ $JAVA_HOME "" ""
    deploy_auth_services $JEMBOSS/lib JembossFileAuthServer.xml http://$LOCALHOST:$PORT/ $JAVA_HOME "" ""
  fi

else

  echo
  echo "*** Generating client and server certificates. These are then"
  echo "*** imported into keystores. The keystores act as databases"
  echo "*** for security certificates."
  echo 
  PASSWD=""
  while [ "$PASSWD" = "" ]
  do
    echo "Provide a password (must be at least 6 characters): "
    read PASSWD
  done

  ssl_create_keystore $LOCALHOST $JEMBOSS/resources "server" "tomcat-sv" $PASSWD
  ssl_create_keystore "Client" $JEMBOSS/resources "client" "tomcat-cl" $PASSWD

  ssl_import $JEMBOSS/resources/server.cer $JEMBOSS/resources/client.keystore $PASSWD
  ssl_import $JEMBOSS/resources/client.cer $JEMBOSS/resources/server.keystore $PASSWD

  echo "Tomcat XML deployment descriptors have been created for the Jemboss Server."
  echo "Would you like an automatic deployment of these to be tried (y/n)?"
  read DEPLOYSERVICE

  if [ "$DEPLOYSERVICE" = "y" ]; then
    ssl_print_notes $JEMBOSS/resources/server.keystore $TOMCAT_ROOT $PORT
    echo

    VAL=0
    while [ $VAL != "y" ] 
    do
      echo "To continue you must have changed the above files!"
      echo "Have the above files been edited (y/n)?"
      read VAL
    done

    ./tomstart
    sleep 45
    OPT_PROP1="-Djava.protocol.handler.pkgs=com.sun.net.ssl.internal.www.protocol"
    OPT_PROP2="-Djavax.net.ssl.trustStore=$JEMBOSS/resources/client.keystore"

    deploy_auth_services $JEMBOSS/lib JembossAuthServer.xml https://$LOCALHOST:$PORT/ $JAVA_HOME $OPT_PROP1 $OPT_PROP2
    deploy_auth_services $JEMBOSS/lib JembossFileAuthServer.xml https://$LOCALHOST:$PORT/ $JAVA_HOME $OPT_PROP1 $OPT_PROP2
  else
    echo
    echo
    echo "--------------------------------------------------------------"
    echo "                          TODO"
    echo "--------------------------------------------------------------"
    echo
    ssl_print_notes $JEMBOSS/resources/server.keystore $TOMCAT_ROOT $PORT
    echo 
    echo "To deploy the Jemboss server methods use soap admin tool at:"
    echo "https://$LOCALHOST:$PORT/soap "
    echo "Follow the notes at:"
    echo "http://www.uk.embnet.org/Software/EMBOSS/Jemboss/download/setup.html"
  fi
fi

#
# Add classes to Tomcat path
#

if [ ! -d "$TOMCAT_ROOT/classes" ]; then
  echo "WARNING:  $TOMCAT_ROOT/classes not found"
  echo "Jemboss classpath not added to Tomcat"

else
  ln -s $JEMBOSS/org $TOMCAT_ROOT/classes
  ln -s $JEMBOSS/resources $TOMCAT_ROOT/classes
fi
#  tomcat_classpath_notes $TOMCAT_ROOT $JEMBOSS

#
#
# Change jemboss.properties to reflect server location
# $EMBOSS_INSTALL/share/EMBOSS/jemboss/resources/jemboss.properties
#
#

echo
echo
echo "--------------------------------------------------------------"
echo "--------------------------------------------------------------"
echo
make_jemboss_properties $EMBOSS_INSTALL $LOCALHOST $AUTH $SSL $PORT

echo
if [ "$SSL" = "y" ]; then
  echo "To secure the admin admin tool https://$LOCALHOST:$PORT/soap "
else
  echo "To secure the admin admin tool http://$LOCALHOST:$PORT/soap "
fi
echo "look at http://www.uk.embnet.org/Software/EMBOSS/Jemboss/download/tomcat.html"
echo
echo "A tomstart and tomstop script to start & stop Tomcat have"
echo "been created in this directory for you convenience."
echo

if [ "$AUTH" = "y" ]; then
  echo 
  echo "You will need to (as root):"
  echo "   chmod u+s $EMBOSS_INSTALL/bin/jembossctl"
  echo "   chown root $EMBOSS_INSTALL/bin/jembossctl"                        
  echo
  echo "Tomcat may still be running! Ensure it is running as the non-priveleged"
  echo "tomcat user. Use the tomstop & tomstart scripts to stop & start tomcat."
  echo 
else
  echo
  echo "Note: Tomcat may still be running!"
  echo "Use the tomstop & tomstart scripts to stop & start tomcat."
  echo
fi

chmod a+x $JEMBOSS/runJemboss.csh

exit 0;


