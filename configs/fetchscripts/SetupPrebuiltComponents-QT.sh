#!/bin/sh

OutDir="QT"
DirectorY="${OutDir}-4.7.4"
FileName="${DirectorY}.tgz"
BuiltDLP="http://downloads.sourceforge.net/project/warzone2100/build-tools/mac/${FileName}"
MD5Sum="74e410b376e1c19b0990f637f646f26e"

configs/FetchPrebuilt.sh "${DirectorY}" "${OutDir}" "${FileName}" "${BuiltDLP}" "${MD5Sum}"
exit ${?}

# tar -czf QT-4.7.4.tgz --exclude '.DS_Store' QT-4.7.4
