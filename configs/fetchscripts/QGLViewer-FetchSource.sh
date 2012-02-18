#!/bin/sh

OutDir="libQGLViewer"
DirectorY="${OutDir}-2.3.10"
FileName="${DirectorY}.tar.gz"
SourceDLP="http://www.libqglviewer.com/src/${FileName}"
MD5Sum="259538c7036b4ae5fe3ed5d4299c8d31"

configs/FetchSource.sh "${DirectorY}" "${OutDir}" "${FileName}" "${SourceDLP}" "${MD5Sum}"
exit ${?}
