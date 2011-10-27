/**
 * @file utilities/save_restore_utility.cpp
 * @author Neil Slagle
 *
 * The SaveRestoreUtility provides helper functions in saving and
 *   restoring models.  The current output file type is XML.
 *
 * @experimental
 */
#include "save_restore_utility.hpp"

using namespace mlpack;
using namespace utilities;

bool SaveRestoreUtility::ReadFile (std::string filename)
{
  xmlDocPtr xmlDocTree = NULL;
  if (NULL == (xmlDocTree = xmlReadFile (filename.c_str(), NULL, 0)))
  {
    errx (1, "Clearly, we couldn't load the XML file\n");
  }
  xmlNodePtr root = xmlDocGetRootElement (xmlDocTree);
  parameters.clear();

  RecurseOnNodes (root->children);
  xmlFreeDoc (xmlDocTree);
  return true;
}
void SaveRestoreUtility::RecurseOnNodes (xmlNode* n)
{
  xmlNodePtr current = NULL;
  for (current = n; current; current = current->next)
  {
    if (current->type == XML_ELEMENT_NODE)
    {
      xmlChar* content = xmlNodeGetContent (current);
      parameters[(const char*) current->name] = (const char*) content;
      xmlFree (content);
    }
    RecurseOnNodes (current->children);
  }
}
bool SaveRestoreUtility::WriteFile (std::string filename)
{
  bool success = false;
  xmlDocPtr xmlDocTree = xmlNewDoc (BAD_CAST "1.0");
  xmlNodePtr root = xmlNewNode(NULL, BAD_CAST "root");
  xmlNodePtr child = NULL;

  xmlDocSetRootElement (xmlDocTree, root);

  for (std::map<std::string, std::string>::iterator it = parameters.begin();
       it != parameters.end();
       ++it)
  {
    child = xmlNewChild (root, NULL,
                         BAD_CAST (*it).first.c_str(),
                         BAD_CAST (*it).second.c_str());
    /* TODO: perhaps we'll add more later?
     * xmlNewProp (child, BAD_CAST "attr", BAD_CAST "add more addibutes?"); */
  }
  /* save the file */
  xmlSaveFormatFileEnc (filename.c_str(), xmlDocTree, "UTF-8", 1);
  xmlFreeDoc (xmlDocTree);
  return success;
}
arma::mat& SaveRestoreUtility::LoadParameter (arma::mat& matrix, std::string name)
{
  std::map<std::string, std::string>::iterator it = parameters.find (name);
  if (it != parameters.end ())
  {
    std::string value = (*it).second;
    boost::char_separator<char> sep ("\n");
    boost::tokenizer<boost::char_separator<char> > tok (value, sep);
    std::list<std::list<double> > rows;
    for (boost::tokenizer<boost::char_separator<char> >::iterator
           tokIt = tok.begin ();
         tokIt != tok.end ();
         ++tokIt)
    {
      std::string row = *tokIt;
      boost::char_separator<char> sepComma (",");
      boost::tokenizer<boost::char_separator<char> >
        tokInner (row, sepComma);
      std::list<double> rowList;
      for (boost::tokenizer<boost::char_separator<char> >::iterator
             tokInnerIt = tokInner.begin ();
             tokInnerIt != tokInner.end ();
             ++tokInnerIt)
      {
        double element;
        std::istringstream iss (*tokInnerIt);
        iss >> element;
        rowList.push_back (element);
      }
      rows.push_back (rowList);
    }
    matrix.zeros (rows.size (), (*(rows.begin ())).size ());
    size_t rowCounter = 0;
    size_t columnCounter = 0;
    for (std::list<std::list<double> >::iterator rowIt = rows.begin ();
         rowIt != rows.end ();
         ++rowIt)
    {
      std::list<double> row = *rowIt;
      columnCounter = 0;
      for (std::list<double>::iterator elementIt = row.begin ();
           elementIt != row.end ();
           ++elementIt)
      {
        matrix(rowCounter, columnCounter) = *elementIt;
        columnCounter++;
      }
      rowCounter++;
    }
    return matrix;
  }
  else
  {
    errx (1, "Missing the correct name\n");
  }
}
std::string SaveRestoreUtility::LoadParameter (std::string str, std::string name)
{
  std::map<std::string, std::string>::iterator it = parameters.find (name);
  if (it != parameters.end ())
  {
    return (*it).second;
  }
  else
  {
    errx (1, "Missing the correct name\n");
  }
}
char SaveRestoreUtility::LoadParameter (char c, std::string name)
{
  int temp;
  std::map<std::string, std::string>::iterator it = parameters.find (name);
  if (it != parameters.end ())
  {
    std::string value = (*it).second;
    std::istringstream input (value);
    input >> temp;
    return (char) temp;
  }
  else
  {
    errx (1, "Missing the correct name\n");
  }
}
void SaveRestoreUtility::SaveParameter (char c, std::string name)
{
  int temp = (int) c;
  std::ostringstream output;
  output << temp;
  parameters[name] = output.str();
}
void SaveRestoreUtility::SaveParameter (arma::mat& mat, std::string name)
{
  std::ostringstream output;
  size_t columns = mat.n_cols;
  size_t rows = mat.n_rows;
  for (size_t r = 0; r < rows; ++r)
  {
    for (size_t c = 0; c < columns - 1; ++c)
    {
      output << mat(r,c) << ",";
    }
    output << mat(r,columns - 1) << std::endl;
  }
  parameters[name] = output.str();
}
