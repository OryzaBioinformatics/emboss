#!/bin/sh
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#
#  @author: Copyright (C) Tim Carver
#
#
# Install EMBOSS & Jemboss 
# last changed: 17/01/03
#
#

######################## Functions ########################


getJavaHomePath()
{
  JAVA_HOME_TMP=${JAVA_HOME_TMP-`which java 2>/dev/null`} 
#  JAVA_HOME_TMP=`which java`

  if [ ! -f "$JAVA_HOME_TMP" ]; then
     if [ -d /usr/java/j2sdk1.4.1 ]; then
       JAVA_HOME_TMP=/usr/java/j2sdk1.4.1
     elif [ -d /usr/local/java/j2sdk1.4.1 ]; then
       JAVA_HOME_TMP=/usr/local/java/j2sdk1.4.1
     else
       JAVA_HOME_TMP=0
     fi
  else
    JAVA_HOME_TMP=`dirname $JAVA_HOME_TMP`
    JAVA_HOME_TMP=`dirname $JAVA_HOME_TMP`
  fi
}


setDataDirectory()
{

  JEMBOSS_CLASS=$1/org/emboss/jemboss/server/
  AUTH=$2
  DATADIR=$3

  OLDPATH="/tmp/SOAP/emboss"

  if [ $AUTH = "y" ]; then
    JEM_CLASS="$JEMBOSS_CLASS/JembossAuthServer.java"
    if test -f "$JEM_CLASS.orig" && (test ! -z "$JEM_CLASS.orig");then
      mv $JEM_CLASS.orig $JEM_CLASS
    fi
    mv $JEM_CLASS $JEM_CLASS.orig
    sed "s|$OLDPATH|$DATADIR|" $JEM_CLASS.orig > $JEM_CLASS
    
    JEM_CLASS="$JEMBOSS_CLASS/JembossFileAuthServer.java"
    if test -f "$JEM_CLASS.orig" && test ! -z "$JEM_CLASS.orig";then
      mv $JEM_CLASS.orig $JEM_CLASS
    fi
    mv $JEM_CLASS $JEM_CLASS.orig
    sed "s|$OLDPATH|$DATADIR|" $JEM_CLASS.orig > $JEM_CLASS
#   echo "sed 's|$OLDPATH|$DATADIR|' $JEM_CLASS.old > $JEM_CLASS"
  else
    JEM_CLASS="$JEMBOSS_CLASS/JembossServer.java"
    if test -f "$JEM_CLASS.orig" && test ! -z "$JEM_CLASS.orig";then
      mv $JEM_CLASS.orig $JEM_CLASS
    fi
    mv $JEM_CLASS $JEM_CLASS.orig
    sed "s|$OLDPATH|$DATADIR|" $JEM_CLASS.orig > $JEM_CLASS

    JEM_CLASS="$JEMBOSS_CLASS/JembossFileServer.java"
    if test -f "$JEM_CLASS.orig" && test ! -z "$JEM_CLASS.orig";then
      mv $JEM_CLASS.orig $JEM_CLASS
    fi
    mv $JEM_CLASS $JEM_CLASS.orig
    sed "s|$OLDPATH|$DATADIR|" $JEM_CLASS.orig > $JEM_CLASS
  fi
 

}

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
   echo "A) EDIT the java.security file "
 fi

 echo "   adding/changing the provider line (usually provider 2 or 3):"
 echo "   security.provider.2=com.sun.net.ssl.internal.ssl.Provider"
 echo

 echo "B) COPY & PASTE THE FOLLOWING INTO (changing port number if required) "$TOMCAT_ROOT/conf/server.xml
 echo

 if [ -d "$TOMCAT_ROOT/shared/classes" ]; then
#tomcat 4.1.x
   echo '    <!-- Define a SSL Coyote HTTP/1.1 Connector on port '$PORT' -->'
   echo '    <Connector className="org.apache.coyote.tomcat4.CoyoteConnector"'
   echo '               port="'$PORT'" minProcessors="5" maxProcessors="75"'
   echo '               enableLookups="false"'
   echo '               acceptCount="10" debug="0" scheme="https" secure="true"'
   echo '               useURIValidationHack="false">'
   echo '      <Factory className="org.apache.coyote.tomcat4.CoyoteServerSocketFactory"'
   echo '           keystoreFile="'$KEYSTOREFILE'" keystorePass="'$PASSWD'"'
   echo '           clientAuth="false" protocol="TLS"/>'
   echo '    </Connector>'
 else
#tomcat 4.0.x
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
 fi

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
  EMBOSS_URL=$6

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
  echo "server.public=$URL/axis/services" \
                                      >> $JEMBOSS_PROPERTIES

  echo "server.private=$URL/axis/services" \
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
  echo "embossURL=$EMBOSS_URL" >> $JEMBOSS_PROPERTIES
  cp $JEMBOSS_PROPERTIES $JEMBOSS_PROPERTIES.bak

  echo
  echo "Changed $EMBOSS_INSTALL/share/EMBOSS/jemboss/resources/jemboss.properties"
  echo "to reflect this installation (original in jemboss.properties.orig)"
  echo

}


deploy_axis_services()
{

  JEMBOSS_LIB=$1
  AXIS=$1/axis
  CLASSPATH=$AXIS/axis.jar::$AXIS/jaxrpc.jar:$AXIS/saaj.jar:$AXIS/commons-logging.jar:
  CLASSPATH=${CLASSPATH}:$AXIS/commons-discovery.jar:$AXIS/wsdl4j.jar:$AXIS/servlet.jar
  CLASSPATH=${CLASSPATH}:$JEMBOSS_LIB/jnet.jar:$JEMBOSS_LIB/jsse.jar:$JEMBOSS_LIB/jcert.jar
  CLASSPATH=${CLASSPATH}:$JEMBOSS_LIB/xerces.jar

  SERVICE=$2
  URL=$3
  JAVAHOME=$4
  OPT_PROP1=$5
  OPT_PROP2=$6

  echo
  echo "Deploying $SERVICE "
  echo "$JAVAHOME/bin/java -classpath $CLASSPATH $OPT_PROP1 $OPT_PROP2 \\ "
  echo "org.apache.axis.client.AdminClient -l$URL/axis/services JembossServer.wsdd"
  echo

  $JAVAHOME/bin/java -classpath $CLASSPATH $OPT_PROP1 $OPT_PROP2 \
        org.apache.axis.client.AdminClient \
        -l$URL/axis/services JembossServer.wsdd

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
  AUTH=$2
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
  echo '<deployment xmlns="http://xml.apache.org/axis/wsdd/"' > $XML_FILE
  echo '            xmlns:java="http://xml.apache.org/axis/wsdd/providers/java">' >> $XML_FILE
  echo "  <service name=\"$ID\" provider=\"java:RPC\">" >> $XML_FILE
  echo "    <parameter name=\"className\" value=\"$JEM_CLASS\"/>" >> $XML_FILE
  echo '    <parameter name="allowedMethods" value="*"/>' >> $XML_FILE
  echo '  </service>' >> $XML_FILE
  echo '  <service name="EmbreoFile" provider="java:RPC">' >> $XML_FILE
  echo "    <parameter name=\"className\" value=\"$FIL_CLASS\"/>" >> $XML_FILE
  echo '    <parameter name="allowedMethods" value="*"/>' >> $XML_FILE
  echo '  </service>' >> $XML_FILE
  echo '</deployment>' >> $XML_FILE

}

echo
echo
echo "--------------------------------------------------------------"
echo " "
echo "         Type of installation [1] :"
echo "         (1) CLIENT-SERVER"
echo "         (2) STANDALONE"
echo " "
echo "--------------------------------------------------------------"
read INSTALL_TYPE
if [ "$INSTALL_TYPE" != "1" ]; then
  if [ "$INSTALL_TYPE" != "2" ]; then
    INSTALL_TYPE="1"
  fi
fi
clear

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

if [ $INSTALL_TYPE = "1" ]; then
  echo "(2) Tomcat release http://jakarta.apache.org/site/binindex.html"
  echo "(3) Apache AXIS (SOAP) release   http://xml.apache.org/axis/"
fi
  
echo
echo "Has the above been downloaded (y/n)? "
read DOWNLOADED

if [ "$DOWNLOADED" != "y" ]; then
  if [ "$DOWNLOADED" != "Y" ]; then
    exit 1
  fi
fi


RECORD="install.record"
if [ -f "$RECORD" ]; then
  mv $RECORD $RECORD.old
fi

echo "$DOWNLOADED" > $RECORD
 
PLATTMP=`uname`

case $PLATTMP in
  Linux)
    PLATTMP="1"
    ;;
  AIX)
    PLATTMP="2"
    ;;
  IRIX)
    PLATTMP="3"
    ;;
  HP-UX)
    PLATTMP="4"
    ;;
  SunOS)
    PLATTMP="5"
    ;;
  Darwin)
    PLATTMP="6"
    ;;
  OSF1)
    PLATTMP="7"
    ;;
  *)
    PLATTMP="1"
    ;;
esac


echo 
echo "Select your platform from 1-6 [$PLATTMP]:"
echo "(1)  linux"
echo "(2)  aix"
echo "(3)  irix"
echo "(4)  hp-ux"
echo "(5)  solaris"
echo "(6)  macosX"
echo "(7)  OSF"
read PLAT

if [ "$PLAT" = "" ]; then
  PLAT=$PLATTMP
fi

echo "$PLAT" >> $RECORD

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
elif [ "$PLAT" = "7" ]; then
  PLATFORM="osf"
else
  echo "Platform not selected from 1-6."
  exit 1
fi

SSL="y"
if [ $INSTALL_TYPE = "1" ]; then
#
# localhost name
#
  echo
  echo "Enter IP of server machine [localhost]:"
  read LOCALHOST

  if [ "$LOCALHOST" = "" ]; then
    LOCALHOST=localhost
  fi

  echo "$LOCALHOST" >> $RECORD
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

  echo "$SSL" >> $RECORD

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

  echo "$PORT" >> $RECORD

fi

#
# JAVA_HOME
#
getJavaHomePath
JAVA_HOME=$JAVA_HOME_TMP
if [ "$JAVA_HOME" != "0" ]; then
  echo "Enter java (1.3 or above) location [$JAVA_HOME_TMP]: "
  read JAVA_HOME

  if [ "$JAVA_HOME" = "" ]; then 
    JAVA_HOME=$JAVA_HOME_TMP
  fi
fi

while [ ! -f "$JAVA_HOME/bin/javac" ]
do
  echo "Enter java (1.3 or above) location (/usr/java/jdk1.3.1/): "
  read JAVA_HOME
done


echo "$JAVA_HOME" >> $RECORD

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
elif [ -d $JAVA_INCLUDE/alpha ]; then
  JAVA_INCLUDE_OS=${JAVA_INCLUDE}/alpha
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

  echo "$JAVA_INCLUDE" >> $RECORD
fi

#
# EMBOSS_DOWNLOAD
#
EMBOSS_DOWNLOAD_TMP=$PWD
EMBOSS_DOWNLOAD_TMP=`dirname $EMBOSS_DOWNLOAD_TMP`
EMBOSS_DOWNLOAD_TMP=`dirname $EMBOSS_DOWNLOAD_TMP`

echo "Enter EMBOSS download directory"
echo "[$EMBOSS_DOWNLOAD_TMP]: "
read EMBOSS_DOWNLOAD

if [ "$EMBOSS_DOWNLOAD" = "" ]; then
  EMBOSS_DOWNLOAD=$EMBOSS_DOWNLOAD_TMP
fi

while [ ! -d "$EMBOSS_DOWNLOAD/ajax" ]
do
  echo "Enter EMBOSS download directory (e.g. /usr/emboss/EMBOSS-2.x.x): "
  read EMBOSS_DOWNLOAD
done

echo "$EMBOSS_DOWNLOAD" >> $RECORD

echo "Enter where EMBOSS should be installed [/usr/local/emboss]: "
read EMBOSS_INSTALL

if [ "$EMBOSS_INSTALL" = "" ]; then
  EMBOSS_INSTALL=/usr/local/emboss
fi

echo "$EMBOSS_INSTALL" >> $RECORD

if [ $INSTALL_TYPE = "1" ]; then
  echo "Enter URL for emboss documentation for application "
  echo "[http://www.uk.embnet.org/Software/EMBOSS/Apps/]:"
  read EMBOSS_URL

  echo "$EMBOSS_URL" >> $RECORD
fi

if [ "$EMBOSS_URL" = "" ]; then
  EMBOSS_URL="http://www.uk.embnet.org/Software/EMBOSS/Apps/"
fi

#
# set JSSE_HOME to the EMBOSS install dir
#
JSSE_HOME=$EMBOSS_INSTALL/share/EMBOSS/jemboss
JEMBOSS_SERVER_AUTH=""
AUTH=y

if [ $INSTALL_TYPE = "1" ]; then
  echo "Enter if you want Jemboss to use unix authorisation (y/n) [y]? "
  read AUTH

  echo "$AUTH" >> $RECORD
else
  AUTH="n"
fi

if [ "$AUTH" = "" ]; then
  AUTH="y"
fi


if [ "$AUTH" = "y" ]; then

  echo "#include <stdio.h>" > dummy.c
  echo 'int main(){ printf("%d",getuid()); }' >> dummy.c
  if (cc dummy.c -o dummy >/dev/null 2>&1); then
    UUIDTMP=`./dummy`
    CC="cc"; 
  else
    gcc dummy.c -o dummy >/dev/null 2>&1
    UUIDTMP=`./dummy`
    CC="gcc";
  fi
  rm -f dummy.c dummy

  if [ "$UUIDTMP" = "" ]; then
    UUIDTMP="506"
  fi

  if [ "$UUIDTMP" = "0" ]; then
    UUIDTMP="506"
  fi

 
  echo "Provide the UID of the account (non-priveleged) to run Tomcat,"
  echo "it has to be greater than 100 [$UUIDTMP]:"
  read UUID

  echo "$UUID" >> $RECORD

  if [ "$UUID" = "" ]; then
    UUID="$UUIDTMP"
  fi

  CC="$CC -DTOMCAT_UID=$UUID "; export CC

  echo
  echo
  echo "(1) shadow      (3) PAM         (5) HP-UX shadow"
  echo "(2) no shadow   (4) AIX shadow  (6) Re-entrant shadow"
  echo "(7) Pain re-entrant"  
  echo 
  echo "Type of unix password method being used "
  echo "(select 1, 2, 3, 4, 5, 6 or 7 )[1]"

  read AUTH_TYPE
  
  echo "$AUTH_TYPE" >> $RECORD

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
 
if [ $INSTALL_TYPE = "1" ]; then
#
#
# SOAP data directory store
#

  echo "Define the directory you want to store the results in"
  echo "[/tmp/SOAP/emboss]"
  read DATADIR
  echo "$DATADIR" >> $RECORD

  if [ "$DATADIR" != "" ]; then
    setDataDirectory $EMBOSS_DOWNLOAD/jemboss $AUTH $DATADIR
  else
    setDataDirectory $EMBOSS_DOWNLOAD/jemboss $AUTH /tmp/SOAP/emboss
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
  echo "$TOMCAT_ROOT" >> $RECORD

#
# Apache AXIS (SOAP)
#
  SOAP_ROOT=0

  while [ ! -d "$SOAP_ROOT/webapps/axis" ]
  do
    echo "Enter Apache AXIS (SOAP) root directory (e.g. /usr/local/xml-axis-xx)"
    read SOAP_ROOT
  done

  echo "$SOAP_ROOT" >> $RECORD

  cp -R $SOAP_ROOT/webapps/axis $TOMCAT_ROOT/webapps
fi
#
#
#

USER_CONFIG=""
echo "Enter any other configuration options (e.g. --with-pngdriver=pathname"
echo "or press return to leave blank):"
read USER_CONFIG

echo "$USER_CONFIG" >> $RECORD
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

#
# create wossname.jar
#
PATH=$PATH:$EMBOSS_INSTALL/bin
export PATH
$EMBOSS_INSTALL/bin/wossname -colon -gui -outf wossname.out -auto
$JAVA_HOME/bin/jar cvf $JEMBOSS/resources/wossname.jar wossname.out

#
#
#

if [ $AUTH = "y" ]; then
  $JAVA_HOME/bin/javac -classpath $JEMBOSS $JEMBOSS/org/emboss/jemboss/server/JembossAuthServer.java
  $JAVA_HOME/bin/javac -classpath $JEMBOSS $JEMBOSS/org/emboss/jemboss/server/JembossFileAuthServer.java
else
  $JAVA_HOME/bin/javac -classpath $JEMBOSS $JEMBOSS/org/emboss/jemboss/server/JembossServer.java
  $JAVA_HOME/bin/javac -classpath $JEMBOSS $JEMBOSS/org/emboss/jemboss/server/JembossFileServer.java
fi

if [ "$MACOSX" = "y" ]; then
  cd $EMBOSS_INSTALL/lib/
  ln -s libajax.dylib libajax.jnilib
# ln -s $EMBOSS_INSTALL/lib/libajax.dylib $EMBOSS_INSTALL/lib/libajax.jnilib
# cp $EMBOSS_INSTALL/lib/libajax.[0-9].dylib /System/Library/Frameworks/JavaVM.framework/Libraries/libajax.jnilib
fi

cd $WORK_DIR

if [ "$PLATFORM" = "hpux" ]; then
  ln -s $EMBOSS_INSTALL/lib/libajax.sl $EMBOSS_INSTALL/lib/libajax.so
fi


#
# Exit if standalone compilation

make_jemboss_properties $EMBOSS_INSTALL $LOCALHOST $AUTH $SSL $PORT $EMBOSS_URL
if [ $INSTALL_TYPE = "2" ]; then
  RUNFILE=$JEMBOSS/runJemboss.csh
  if [ -f "$RUNFILE.bak" ]; then
    rm -f $RUNFILE.bak
  fi
  sed "s|^#java org|java org|" $RUNFILE > $RUNFILE.new
  sed "s|^java org.emboss.jemboss.Jemboss &|#java org.emboss.jemboss.Jemboss &|" $RUNFILE.new  > $RUNFILE.new1
  sed "s|^#setenv EMBOSS_INSTALL.*|setenv EMBOSS_INSTALL $EMBOSS_INSTALL/lib|"   $RUNFILE.new1 > $RUNFILE.new2
  sed "s|^#setenv LD_LIBRARY_PATH|setenv LD_LIBRARY_PATH|"                       $RUNFILE.new2 > $RUNFILE.new3
  rm -f $RUNFILE.new $RUNFILE.new1 $RUNFILE.new2
  mv $RUNFILE $RUNFILE.bak
  mv $RUNFILE.new3 $RUNFILE
  chmod a+x $RUNFILE
  exit 0
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
 echo "setenv JAVA_OPTS \"-Djava.library.path=$EMBOSS_INSTALL/lib\"" >> tomstart
 echo "setenv LD_LIBRARY_PATH $EMBOSS_INSTALL/lib" >> tomstart
fi

if [ "$MACOSX" = "y" ]; then
 echo "setenv DYLD_LIBRARY_PATH $EMBOSS_INSTALL/lib" >> tomstart
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
#if [ "$AUTH_TYPE" = "3" ]; then
#  echo "setenv LD_PRELOAD /lib/libpam.so" >> tomstop
#fi
echo 'set path=($path '"$JAVA_HOME/bin)"  >> tomstop
echo "rehash"  >> tomstop
echo "$TOMCAT_ROOT/bin/shutdown.sh"  >> tomstop


chmod u+x tomstart
chmod u+x tomstop

#
# Add classes to Tomcat path
#

if [ -d "$TOMCAT_ROOT/shared/classes" ]; then
#tomcat 4.1.x
  cp $JEMBOSS/lib/mail.jar $TOMCAT_ROOT/webapps/axis/WEB-INF/lib
  cp $JEMBOSS/lib/activation.jar $TOMCAT_ROOT/webapps/axis/WEB-INF/lib

  mv $JEMBOSS/org $TOMCAT_ROOT/webapps/axis/WEB-INF/classes/org
  mv $JEMBOSS/resources $TOMCAT_ROOT/webapps/axis/WEB-INF/classes/resources

  ln -s $TOMCAT_ROOT/webapps/axis/WEB-INF/classes/org $JEMBOSS/org
  ln -s $TOMCAT_ROOT/webapps/axis/WEB-INF/classes/resources $JEMBOSS/resources
  
  cp -R $EMBOSS_DOWNLOAD/jemboss/lib/axis $JEMBOSS/lib
  
# logging jar need moving
  mv $TOMCAT_ROOT/webapps/axis/WEB-INF/lib/log4j-1.2.4.jar $TOMCAT_ROOT/server/lib
else
  echo "WARNING: no $TOMCAT_ROOT/shared/classes "
  echo "Jemboss classpath not added to Tomcat"
fi


#
#
# Create XML deployment descriptor files
#

#JEMBOSS=$EMBOSS_INSTALL/share/EMBOSS/jemboss
output_auth_xml JembossServer.wsdd $AUTH

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
#   deploy_axis_services $JEMBOSS/lib JembossServer.wsdd http://$LOCALHOST:$PORT/ $JAVA_HOME "" ""
    deploy_axis_services $JEMBOSS/lib JembossServer.wsdd http://localhost:$PORT/ $JAVA_HOME "" ""
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
    echo
    echo "Please wait, starting tomcat......."

    sleep 45
    OPT_PROP1="-Djava.protocol.handler.pkgs=com.sun.net.ssl.internal.www.protocol"
    OPT_PROP2="-Djavax.net.ssl.trustStore=$JEMBOSS/resources/client.keystore"

#   deploy_axis_services $JEMBOSS/lib JembossServer.wsdd https://$LOCALHOST:$PORT/ $JAVA_HOME $OPT_PROP1 $OPT_PROP2
    deploy_axis_services $JEMBOSS/lib JembossServer.wsdd https://localhost:$PORT/ $JAVA_HOME $OPT_PROP1 $OPT_PROP2
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
# Change jemboss.properties to reflect server location
# $EMBOSS_INSTALL/share/EMBOSS/jemboss/resources/jemboss.properties
#
#

echo
echo
echo "--------------------------------------------------------------"
echo "--------------------------------------------------------------"
echo
#make_jemboss_properties $EMBOSS_INSTALL $LOCALHOST $AUTH $SSL $PORT $EMBOSS_URL

#if [ "$SSL" = "y" ]; then
#  echo "To secure the admin admin tool https://$LOCALHOST:$PORT/soap "
#else
#  echo "To secure the admin admin tool http://$LOCALHOST:$PORT/soap "
#fi
#echo "look at http://www.uk.embnet.org/Software/EMBOSS/Jemboss/download/tomcat.html"
echo
echo "A tomstart and tomstop script to start & stop Tomcat have"
echo "been created. It important to use these to start & stop Tomcat."

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

if [ ! -d "$DATADIR" ]; then
  echo "Create the user results directory (and ensure this is world read/write-able): "
  echo "   mkdir $DATADIR"
  echo
fi
echo "Try running Jemboss with the script:"
echo "   cd $JEMBOSS"
echo "   ./runJemboss.csh"
echo

chmod a+x $JEMBOSS/runJemboss.csh
chmod u+x $JEMBOSS/utils/*sh

exit 0;


