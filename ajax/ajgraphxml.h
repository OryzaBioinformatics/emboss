#ifdef GROUT
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ajgraphxml_h
#define ajgraphxml_h

#include <gdome.h>

/* @data AjPXmlNode ***********************************************************
**
** Xml graph node
**
** @alias AjSXmlNode
** @alias AjOXmlNode
**
** @attr theNode [GdomeNode] Undocumented
******************************************************************************/

typedef struct AjSXmlNode
{
    GdomeNode *theNode;
} AjOXmlNode, *AjPXmlNode;

/* @data AjPXmlFile ***********************************************************
**
** Xml graph file
**
** @alias AjSXmlFile
** @alias AjOXmlFile
**
** @attr domimpl [GdomeDOMImplementation *] Undocumented
** @attr doc [GdomeDocument *] Undocumented
** @attr currentGraphic [AjPXmlNode] Undocumented
** @attr currentScene [AjPXmlNode] Undocumented
** @attr colour [double[3]] Undocumented
** @attr nodeTypes [AjPTable] Undocumented
******************************************************************************/

typedef struct AjSXmlFile
{
    GdomeDOMImplementation *domimpl;
    GdomeDocument *doc;
    AjPXmlNode currentGraphic;
    AjPXmlNode currentScene;
    double colour[3];
    AjPTable nodeTypes;
} AjOXmlFile, *AjPXmlFile;


AjPXmlNode	ajXmlNodeNew();
AjPXmlFile 	ajXmlCreateNewOutputFile();
AjPXmlFile 	ajXmlFileNew();

void 	ajXmlNodeDel(AjPXmlNode *thys);

void 	ajXmlFileDel(AjPXmlFile *thys);

AjBool 	ajXmlSetMaxMin(AjPXmlFile file, double xMin, double yMin,
			       double xMax, double yMax);
AjBool 	ajXmlWriteFile(AjPXmlFile file, AjPStr filename);
AjBool 	ajXmlWriteStdout(AjPXmlFile file);
void 	ajXmlClearFile(AjPXmlFile file);
AjBool 	ajXmlSetSource(AjPXmlFile file, AjPStr title);
AjBool 	ajXmlAddMainTitleC(AjPXmlFile file, char *title);
AjBool 	ajXmlAddXTitleC (AjPXmlFile file, char *title);
AjBool 	ajXmlAddYTitleC (AjPXmlFile file, char *title);
AjBool 	ajXmlAddMainTitle(AjPXmlFile file, AjPStr title);
AjBool 	ajXmlAddXTitle(AjPXmlFile file, AjPStr title);
AjBool 	ajXmlAddYTitle(AjPXmlFile file, AjPStr title);
void 	ajXmlAddText(AjPXmlFile file, double x, double y,
		     double size, double angle, AjPStr fontFamily,
		     AjPStr fontStyle, AjPStr text);
void 	ajXmlAddTextC(AjPXmlFile file, double x, double y,
		      double size, double angle, char *fontFamily,
		      char *fontStyle, char *text);
void 	ajXmlAddTextCentred(AjPXmlFile file, double x, double y,
			    double size, double angle,
			    AjPStr fontFamily, AjPStr fontStyle,
			    AjPStr text);
void 	ajXmlAddTextWithJustify(AjPXmlFile file, double x, double y,
				double size, double angle,
				AjPStr fontFamily, AjPStr fontStyle,
				AjPStr text, AjBool horizontal,
				AjBool leftToRight,
				AjBool topToBottom,
				AjPStr justifyMajor,
				AjPStr justifyMinor);
void 	ajXmlAddTextWithCJustify(AjPXmlFile file, double x, double y,
				 double size, double angle,
				 AjPStr fontFamily, AjPStr fontStyle,
				 AjPStr text, AjBool horizontal,
				 AjBool leftToRight,
				 AjBool topToBottom,
				 char *justifyMajor,
				 char *justifyMinor);
void 	ajXmlAddTextOnArc(AjPXmlFile file, double xCentre,
			  double yCentre, double startAngle,
			  double endAngle, double radius,
			  double size, AjPStr fontFamily,
			  AjPStr fontStyle, AjPStr text);
void 	ajXmlAddJoinedLineSetEqualGapsF(AjPXmlFile file, float *y,
					int numberOfPoints,
					float startX, float increment);
void 	ajXmlAddJoinedLineSet(AjPXmlFile file, double *x, double *y,
			      int numberOfPoints);
void 	ajXmlAddJoinedLineSetF(AjPXmlFile file, float *x, float *y,
			       int numberOfPoints);
void 	ajXmlAddLine(AjPXmlFile file, double x1, double y1, double x2,
		     double y2);
void 	ajXmlAddLineF(AjPXmlFile file, float x1, float y1, float x2, float y2);
void 	ajXmlAddPoint(AjPXmlFile file, double x1, double y1);
void 	ajXmlAddRectangle(AjPXmlFile file, double x1, double y1, double x2,
			  double y2, AjBool fill);
void 	ajXmlAddHistogramEqualGapsF(AjPXmlFile file, float *y, int numPoints,
				    float startX, float xGap);
void 	ajXmlAddRectangleSet(AjPXmlFile file, double *x1, double *y1,
			     double *x2, double *y2, int numPoints,
			     AjBool fill);
void 	ajXmlAddCylinder(AjPXmlFile file, double x1, double y1, double x2,
			 double y2, double width);
AjBool 	ajXmlAddPointLabelCircle(AjPXmlFile file, double angle,
				 double xCentre, double yCentre,
				 double radius, double length, double size,
				 AjPStr fontFamily, AjPStr fontStyle,
				 AjPStr text);
AjBool 	ajXmlAddSectionLabelCircle(AjPXmlFile file, double startAngle,
				   double endAngle, double xCentre,
				   double yCentre, double radius,
				   double width, double labelArmAngle,
				   AjPStr labelStyle, double textPosition,
				   double size, AjPStr fontFamily,
				   AjPStr fontStyle, AjPStr text);
AjBool 	ajXmlAddPointLabelLinear(AjPXmlFile file, double angle, double xPoint,
				 double yPoint, double length,
				 AjBool textParallelToLine, double size,
				 AjPStr fontFamily, AjPStr fontStyle,
				 AjPStr text);
AjBool 	ajXmlAddSectionLabelLinear(AjPXmlFile file, double xStart,
				   double yStart, double xEnd, double yEnd,
				   double width, double labelArmLength,
				   AjPStr labelStyle, double textPosition,
				   double size, AjPStr fontFamily,
				   AjPStr fontStyle, AjPStr text);
AjBool 	ajXmlAddSquareResidueLinear(AjPXmlFile file, char residue, float x,
				    float y);
AjBool 	ajXmlAddOctagonalResidueLinear(AjPXmlFile file, char residue, float x,
				       float y);
AjBool 	ajXmlAddDiamondResidueLinear(AjPXmlFile file, char residue, float x,
				     float y);
AjBool 	ajXmlAddNakedResidueLinear(AjPXmlFile file, char residue, float x,
				   float y);
AjBool 	ajXmlAddSquareResidue(AjPXmlFile file, char residue, double radius,
			      double angle);
AjBool 	ajXmlAddOctagonalResidue(AjPXmlFile file, char residue, double radius,
				 double angle);
AjBool 	ajXmlAddDiamondResidue(AjPXmlFile file, char residue, double radius,
			       double angle);
AjBool 	ajXmlAddNakedResidue(AjPXmlFile file, char residue, double radius,
			     double angle);
float 	ajXmlFitTextOnLine(float x1, float y1, float x2, float y2,
			   AjPStr text);
void 	ajXmlGetColour(AjPXmlFile file, double r, double g, double b);
void 	ajXmlSetColour(AjPXmlFile file, double r, double g, double b);
void 	ajXmlSetColourFromCode(AjPXmlFile file, ajint colour);
void 	ajXmlAddGraphic(AjPXmlFile file, AjPStr type);
void 	ajXmlAddGraphicC(AjPXmlFile file, char *type);

void 	ajXmlAddCircle(AjPXmlFile file, double xCentre, double yCentre,
		       double radius);
void 	ajXmlAddCircleF(AjPXmlFile file, float xCentre, float yCentre,
			float radius);
void 	ajXmlAddArc(AjPXmlFile file, double xCentre, double yCentre,
		    double startAngle, double endAngle, double radius);
void 	ajXmlAddGroutOption(AjPXmlFile file, AjPStr name, AjPStr value);
void 	ajXmlAddGroutOptionC(AjPXmlFile file, char *name, char *value);


#endif

#ifdef __cplusplus
}
#endif
#endif
