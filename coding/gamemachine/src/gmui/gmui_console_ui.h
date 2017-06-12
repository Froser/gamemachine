BEGIN_NS

namespace gmui
{
	struct GMUIConsole
	{
		static const GMWchar* UI;
	};
}

const GMWchar* gmui::GMUIConsole::UI = _L(""
	"<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?> "
	"<Window size=\"700,700\"> "
	"    <VerticalLayout enabled=\"true\" bkcolor=\"#FFA6CAF0\"> "
	"        <HorizontalLayout enabled=\"true\" width=\"700\" height=\"43\"> "
	"            <Option text=\"Log\" float=\"true\" pos=\"8,7,0,0\" enabled=\"true\" width=\"60\" height=\"30\" textcolor=\"#FF000000\" disabledtextcolor=\"#FFA7A6AA\" align=\"center\" /> "
	"            <Option text=\"Performance\" float=\"true\" pos=\"83,7,0,0\" enabled=\"true\" width=\"114\" height=\"30\" textcolor=\"#FF000000\" disabledtextcolor=\"#FFA7A6AA\" align=\"center\" /> "
	"        </HorizontalLayout> "
	"        <HorizontalLayout enabled=\"true\" width=\"700\"> "
	"            <TabLayout name=\"Switch\" enabled=\"true\" width=\"698\" height=\"500\"> "
	"                <HorizontalLayout enabled=\"true\" width=\"698\" height=\"500\"> "
	"                    <RichEdit name=\"Edit_Console\" float=\"true\" pos=\"100,10,650,810\" enabled=\"true\" width=\"550\" height=\"500\" bkcolor=\"#FFFFFFFF\" autovscroll=\"true\" vscrollbar=\"true\"/> "
	"                </HorizontalLayout> "
	"                <HorizontalLayout enabled=\"true\" width=\"698\" height=\"500\" /> "
	"            </TabLayout> "
	"        </HorizontalLayout> "
	"    </VerticalLayout> "
	"</Window> "
	;
);
END_NS