BEGIN_NS

namespace gmui
{
	struct GMUIConsole
	{
		static const GMWchar* UI;
	};
}

const GMWchar* gmui::GMUIConsole::UI = _L(""
	L"<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?> "
	L"<Window size=\"700,700\"> "
	L"    <VerticalLayout enabled=\"true\" bkcolor=\"#FFA6CAF0\"> "
	L"        <HorizontalLayout enabled=\"true\" width=\"700\" height=\"43\"> "
	L"            <Option text=\"Log\" float=\"true\" pos=\"8,7,0,0\" enabled=\"true\" width=\"60\" height=\"30\" textcolor=\"#FF000000\" disabledtextcolor=\"#FFA7A6AA\" align=\"center\" /> "
	L"            <Option text=\"Performance\" float=\"true\" pos=\"83,7,0,0\" enabled=\"true\" width=\"114\" height=\"30\" textcolor=\"#FF000000\" disabledtextcolor=\"#FFA7A6AA\" align=\"center\" /> "
	L"        </HorizontalLayout> "
	L"        <HorizontalLayout enabled=\"true\" width=\"700\"> "
	L"            <TabLayout name=\"Switch\" enabled=\"true\" width=\"698\" height=\"500\"> "
	L"                <HorizontalLayout enabled=\"true\" width=\"698\" height=\"500\"> "
	L"                    <RichEdit name=\"Edit_Console\" float=\"true\" pos=\"100,10,650,810\" enabled=\"true\" width=\"550\" height=\"500\" bkcolor=\"#FFFFFFFF\" autovscroll=\"true\" vscrollbar=\"true\"/> "
	L"                </HorizontalLayout> "
	L"                <HorizontalLayout enabled=\"true\" width=\"698\" height=\"500\" /> "
	L"            </TabLayout> "
	L"        </HorizontalLayout> "
	L"    </VerticalLayout> "
	L"</Window> "
);
END_NS