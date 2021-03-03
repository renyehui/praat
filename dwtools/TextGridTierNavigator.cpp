/* TextGridTierNavigator.cpp
 *
 * Copyright (C) 2021 David Weenink
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this work. If not, see <http://www.gnu.org/licenses/>.
 */

#include "TextGridTierNavigator.h"

#include "enums_getText.h"
#include "TextGridTierNavigator_enums.h"
#include "enums_getValue.h"
#include "TextGridTierNavigator_enums.h"

#include "oo_DESTROY.h"
#include "TextGridTierNavigator_def.h"
#include "oo_COPY.h"
#include "TextGridTierNavigator_def.h"
#include "oo_EQUAL.h"
#include "TextGridTierNavigator_def.h"
#include "oo_CAN_WRITE_AS_ENCODING.h"
#include "TextGridTierNavigator_def.h"
#include "oo_WRITE_TEXT.h"
#include "TextGridTierNavigator_def.h"
#include "oo_READ_TEXT.h"
#include "TextGridTierNavigator_def.h"
#include "oo_WRITE_BINARY.h"
#include "TextGridTierNavigator_def.h"
#include "oo_READ_BINARY.h"
#include "TextGridTierNavigator_def.h"
#include "oo_DESCRIPTION.h"
#include "TextGridTierNavigator_def.h"

Thing_implement (TextGridTierNavigator, Function, 0);

static integer IntervalTier_getSize (Function tier) {
	IntervalTier me = reinterpret_cast<IntervalTier> (tier);
	return my intervals.size;
}

static double IntervalTier_getStartTime (Function tier, integer index) {
	IntervalTier me = reinterpret_cast<IntervalTier> (tier);
	if (index < 1 || index > my intervals.size)
		return undefined;
	TextInterval interval = my intervals.at [index];
	return interval -> xmin;
}
	
static double IntervalTier_getEndTime (Function tier, integer index) {
	IntervalTier me = reinterpret_cast<IntervalTier> (tier);
	if (index < 1 || index > my intervals.size)
		return undefined;
	TextInterval interval = my intervals.at [index];
	return interval -> xmax;
}
	
static conststring32 IntervalTier_getLabel (Function tier, integer index) {
	IntervalTier me = reinterpret_cast<IntervalTier> (tier);
	if (index < 1 || index > my intervals.size)
		return U"-- undefined --";
	TextInterval interval = my intervals.at [index];
	return interval -> text.get();
}

static integer TextTier_getSize (Function tier) {
	TextTier me = reinterpret_cast<TextTier> (tier);
	return my points.size;
}

static double TextTier_getStartTime (Function tier, integer index) {
	TextTier me = reinterpret_cast<TextTier> (tier);
	if (index < 1 || index > my points.size)
		return undefined;
	TextPoint point = my points.at [index];
	return point -> number;
}

static double TextTier_getEndTime (Function tier, integer index) {
	TextTier me = reinterpret_cast<TextTier> (tier);
	if (index < 1 || index > my points.size)
		return undefined;
	TextPoint point = my points.at [index];
	return point -> number;;
}

static conststring32 TextTier_getLabel (Function tier, integer index) {
	TextTier me = reinterpret_cast<TextTier> (tier);
	if (index < 1 || index > my points.size)
		return U"-- undefined --";
	TextPoint point = my points.at [index];
	return point -> mark.get();
}

integer structTextGridTierNavigator :: v_getSize () {
	return ( tier -> classInfo == classIntervalTier ? IntervalTier_getSize (tier.get()) : 
		TextTier_getSize (tier.get()) );
}

integer structTextGridTierNavigator :: v_timeToLowIndex (double time) {
	return ( tier -> classInfo == classIntervalTier ? 
		IntervalTier_timeToLowIndex ((IntervalTier) tier.get(), time) : 
		AnyTier_timeToLowIndex ((AnyTier) tier.get(), time) );
}

integer structTextGridTierNavigator :: v_timeToIndex (double time) {
	return ( tier -> classInfo == classIntervalTier ? IntervalTier_timeToIndex ((IntervalTier) tier.get(), time) : 
		AnyTier_timeToNearestIndex ((AnyTier) tier.get(), time) ); // TODO is that ok?
}

integer structTextGridTierNavigator :: v_timeToHighIndex (double time) {
	return ( tier -> classInfo == classIntervalTier ? IntervalTier_timeToHighIndex ((IntervalTier) tier.get(), time) : 
		AnyTier_timeToHighIndex ((AnyTier)tier.get(), time) );
}

double structTextGridTierNavigator :: v_getStartTime (integer index) {
	return ( tier -> classInfo == classIntervalTier ? IntervalTier_getStartTime (tier.get(), index) : 
		TextTier_getStartTime (tier.get(), index) );
}

double structTextGridTierNavigator :: v_getEndTime (integer index) {
	return ( tier -> classInfo == classIntervalTier ? IntervalTier_getEndTime (tier.get(), index) : 
		TextTier_getEndTime (tier.get(), index) );
}

conststring32 structTextGridTierNavigator :: v_getLabel (integer index) {
	return ( tier -> classInfo == classIntervalTier ? IntervalTier_getLabel (tier.get(), index) : 
		TextTier_getLabel (tier.get(), index) );
}


void structTextGridTierNavigator :: v_info () {
	const integer tierSize = our v_getSize ();
	MelderInfo_writeLine (U"\tNumber of matches on tier: ");
	MelderInfo_writeLine (U"\t\tTopic labels only: ",
		TextGridTierNavigator_getNumberOfTopicMatches (this), U" (from ", tierSize, U")");
	MelderInfo_writeLine (U"\t\tBefore labels only: ",
		TextGridTierNavigator_getNumberOfBeforeMatches (this), U" (from ", tierSize, U")");
	MelderInfo_writeLine (U"\t\tAfter labels only: ",
		TextGridTierNavigator_getNumberOfAfterMatches (this), U" (from ", tierSize, U")");
	MelderInfo_writeLine (U"\t\tCombined: ", TextGridTierNavigator_getNumberOfMatches (this),  U" (from ", tierSize, U")");
	MelderInfo_writeLine (U"\tMatch domain: ", kMatchDomain_getText (our matchDomain));
}

static void NavigationContext_checkMatchDomain (NavigationContext me, kMatchDomain matchDomain) {
	if (matchDomain == kMatchDomain::BEFORE_START_TO_TOPIC_END)
		Melder_require (my useCriterion == kContext_use::BEFORE || my useCriterion != kContext_use::BEFORE_AND_AFTER,
			U"You should not use the match domain <", kMatchDomain_getText (kMatchDomain::BEFORE_START_TO_TOPIC_END), U"> if you don't always use Before in the matching <", kContext_use_getText (my useCriterion), U">.");
	else if (matchDomain == kMatchDomain::BEFORE_START_TO_AFTER_END)
		Melder_require (my useCriterion == kContext_use::BEFORE_AND_AFTER,
			U"You should not use the match domain <", kMatchDomain_getText (kMatchDomain::BEFORE_START_TO_AFTER_END), U"> if you don't always use Before and After in the matching <", kContext_use_getText (my useCriterion), U">.");
	else if (matchDomain == kMatchDomain::TOPIC_START_TO_AFTER_END)
		Melder_require (my useCriterion == kContext_use::AFTER || my useCriterion != kContext_use::BEFORE_AND_AFTER,
			U"You should not use the match domain <", kMatchDomain_getText (kMatchDomain::TOPIC_START_TO_AFTER_END), U"> if you don't always use After in the matching <", kContext_use_getText (my useCriterion), U">.");
	else if (matchDomain == kMatchDomain::BEFORE_START_TO_BEFORE_END)
		Melder_require (my useCriterion == kContext_use::BEFORE || my useCriterion == kContext_use::BEFORE_AND_AFTER,
			U"You should not use the match domain <", kMatchDomain_getText (kMatchDomain::BEFORE_START_TO_BEFORE_END), U"> if you don't always use Before in the matching <", kContext_use_getText (my useCriterion), U">.");
	else if (matchDomain == kMatchDomain::AFTER_START_TO_AFTER_END)
		Melder_require (my useCriterion == kContext_use::AFTER || my useCriterion == kContext_use::BEFORE_AND_AFTER,
			U"You should not use the match domain <", kMatchDomain_getText (kMatchDomain::AFTER_START_TO_AFTER_END), U"> if you don't always use After in the matching <", kContext_use_getText (my useCriterion), U">.");
	// else MATCH_START_TO_MATCH_END || TOPIC_START_TO_TOPIC_END are always ok.
}

void TextGridTierNavigator_getMatchDomain (TextGridTierNavigator me, double *out_startTime, double *out_endTime) {
	kContext_where startWhere, endWhere;
	if (my matchDomain == kMatchDomain::MATCH_START_TO_MATCH_END) {
		const NavigationContext nc = my navigationContext.get();
		if (nc -> useCriterion == kContext_use::NO_BEFORE_AND_NO_AFTER) {
			startWhere = kContext_where::TOPIC;
			endWhere = kContext_where::TOPIC;
		} else if (nc -> useCriterion == kContext_use::BEFORE) {
			startWhere = kContext_where::BEFORE;
			endWhere = ( nc -> excludeTopicMatch ? kContext_where::BEFORE : kContext_where::TOPIC );
		} else if (nc -> useCriterion == kContext_use::AFTER) {
			startWhere = ( nc -> excludeTopicMatch ? kContext_where::AFTER : kContext_where::TOPIC );
			endWhere = kContext_where::AFTER;
		} else if (nc -> useCriterion == kContext_use::BEFORE_AND_AFTER) {
			startWhere = kContext_where::BEFORE;
			endWhere = kContext_where::AFTER;
		} else if (nc -> useCriterion == kContext_use::BEFORE_OR_AFTER_NOT_BOTH) {
			if (TextGridTierNavigator_isBeforeMatch (me, my currentTopicIndex)) {
				startWhere = kContext_where::BEFORE;
				endWhere = ( nc -> excludeTopicMatch ? kContext_where::BEFORE : kContext_where::TOPIC );
			} else {
				startWhere = ( nc -> excludeTopicMatch ? kContext_where::AFTER : kContext_where::TOPIC );
				endWhere = kContext_where::AFTER;
			}
		} else if (nc -> useCriterion == kContext_use::BEFORE_OR_AFTER_OR_BOTH) {
			bool isBeforeMatch = TextGridTierNavigator_isBeforeMatch (me, my currentTopicIndex);
			bool isAfterMatch = TextGridTierNavigator_isAfterMatch (me, my currentTopicIndex);
			if (isBeforeMatch && isAfterMatch) {
				startWhere = kContext_where::BEFORE;
				endWhere = kContext_where::AFTER;
			} else if (isBeforeMatch) {
				startWhere = kContext_where::BEFORE;
				endWhere = ( nc -> excludeTopicMatch ? kContext_where::BEFORE : kContext_where::TOPIC );
			} else {
				startWhere = ( nc -> excludeTopicMatch ? kContext_where::AFTER : kContext_where::TOPIC );
				endWhere = kContext_where::AFTER;
			}
		}
	} else if (my matchDomain == kMatchDomain::TOPIC_START_TO_TOPIC_END) {
		startWhere = kContext_where::TOPIC;
		endWhere = kContext_where::TOPIC;
	} else if (my matchDomain == kMatchDomain::BEFORE_START_TO_TOPIC_END) {
		startWhere = kContext_where::BEFORE;
		endWhere = kContext_where::TOPIC;
	} else if (my matchDomain == kMatchDomain::TOPIC_START_TO_AFTER_END) {
		startWhere = kContext_where::TOPIC;
		endWhere = kContext_where::AFTER;
	} else if (my matchDomain == kMatchDomain::BEFORE_START_TO_AFTER_END) {
		startWhere = kContext_where::BEFORE;
		endWhere = kContext_where::AFTER;
	} else if (my matchDomain == kMatchDomain::BEFORE_START_TO_BEFORE_END) {
		startWhere = kContext_where::BEFORE;
		endWhere = kContext_where::BEFORE;
	} else if (my matchDomain == kMatchDomain::AFTER_START_TO_AFTER_END) {
		startWhere = kContext_where::AFTER;
		endWhere = kContext_where::AFTER;
	}
	if (out_startTime)
		*out_startTime = TextGridTierNavigator_getStartTime (me, startWhere);
	if (out_endTime)
		*out_endTime = TextGridTierNavigator_getEndTime (me, endWhere);
}

autoTextGridTierNavigator TextGridTierNavigator_create (Function me, NavigationContext thee, kMatchDomain matchDomain) {
	try {
		NavigationContext_checkMatchDomain (thee, matchDomain);
		autoTextGridTierNavigator him = Thing_new (TextGridTierNavigator);
		Function_init (him.get(), my xmin, my xmax);  
		his tier = Data_copy (me);
		his navigationContext = Data_copy (thee);
		his beforeRange.first = his beforeRange.last = 1;
		his afterRange.first = his afterRange.last = 1;
		his matchDomain = matchDomain;
		his matchLocation = kMatchLocation::IS_ANYWHERE;
		return him;
	} catch (MelderError) {
		Melder_throw (U"TextGridTierNavigator not created");
	}
}

autoTextGridTierNavigator TextGrid_to_TextGridTierNavigator_topic (TextGrid me, integer tierNumber, conststring32 topic_string, kMelder_string topicCriterion, kMatchBoolean topicMatchBoolean, kMatchDomain matchDomain) {
	try {
		autoNavigationContext navigationContext = NavigationContext_createTopicOnly (topic_string, topicCriterion, topicMatchBoolean);
		autoTextGridTierNavigator thee = TextGrid_and_NavigationContext_to_TextGridTierNavigator (me, navigationContext.get(), tierNumber,  matchDomain);
		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": could not create TextGridTierNavigator.");
	}
}

autoTextGridTierNavigator TextGrid_and_NavigationContext_to_TextGridTierNavigator (TextGrid me, NavigationContext thee, integer tierNumber, kMatchDomain matchDomain) {
	try {
		Function tier = TextGrid_checkSpecifiedTierNumberWithinRange (me, tierNumber);
		autoTextGridTierNavigator him = TextGridTierNavigator_create (tier, thee, matchDomain);
		his tierNumber = tierNumber;
		return him;
	} catch (MelderError) {
		Melder_throw (me, U": could not create TextGridTierNavigator.");
	}	
}

void TextGridTierNavigator_replaceNavigationContext (TextGridTierNavigator me, NavigationContext thee) {
	try {
		my navigationContext -> topicLabels = Data_copy (thy topicLabels.get());
		my navigationContext -> topicCriterion = thy topicCriterion;
		my navigationContext -> topicMatchBoolean = thy topicMatchBoolean;
		my navigationContext -> beforeLabels = Data_copy (thy beforeLabels.get());
		my navigationContext -> beforeCriterion = thy beforeCriterion;
		my navigationContext -> beforeMatchBoolean = thy beforeMatchBoolean;
		my navigationContext -> afterLabels = Data_copy (thy afterLabels.get());
		my navigationContext -> afterCriterion = thy afterCriterion;
		my navigationContext -> afterMatchBoolean = thy afterMatchBoolean;
		my navigationContext -> useCriterion = thy useCriterion;
		my navigationContext -> excludeTopicMatch = thy excludeTopicMatch;		
	} catch (MelderError) {
		Melder_throw (me, U": could not replace navigation context.");
	}
}

autoNavigationContext TextGridTierNavigator_extractNavigationContext (TextGridTierNavigator me) {
	try {
		autoNavigationContext thee = Thing_new (NavigationContext);
		thy topicLabels = Data_copy (my navigationContext -> topicLabels.get());
		thy topicCriterion = my navigationContext -> topicCriterion;
		thy topicMatchBoolean = my navigationContext -> topicMatchBoolean;
		thy beforeLabels = Data_copy (my navigationContext -> beforeLabels.get());
		thy beforeCriterion = my navigationContext -> beforeCriterion;
		thy beforeMatchBoolean = my navigationContext -> beforeMatchBoolean;
		thy afterLabels = Data_copy (my navigationContext -> afterLabels.get());
		thy afterCriterion = my navigationContext -> afterCriterion;
		thy useCriterion = my navigationContext -> useCriterion;
		thy afterMatchBoolean = my navigationContext -> afterMatchBoolean;
		thy excludeTopicMatch = my navigationContext -> excludeTopicMatch;
		return thee;
	} catch (MelderError) {
		Melder_throw (me, U": could not extract navigation context.");
	}
}

void TextGridTierNavigator_replaceTier (TextGridTierNavigator me, TextGrid thee, integer tierNumber) {
	try {
		const Function tier = TextGrid_checkSpecifiedTierNumberWithinRange (thee, tierNumber);
		Melder_require (my tier -> classInfo == tier -> classInfo,
			U"The tier should be of the same type as the one you want to replace.");
		my tier = Data_copy (tier);
		my currentTopicIndex = 0; // offLeft
	} catch (MelderError) {
		Melder_throw (me, U": cannot replace the tier.");
	}
}

void TextGridTierNavigator_modifyBeforeRange (TextGridTierNavigator me, integer from, integer to) {
	Melder_require (from > 0 &&  to > 0,
		U"Both numbers in the Before range should be positive.");
	my beforeRange.first = std::min (from, to);
	my beforeRange.last = std::max (from, to);
}

void TextGridTierNavigator_modifyAfterRange (TextGridTierNavigator me, integer from, integer to) {
	Melder_require (from > 0 &&  to > 0,
		U"Both numbers in the after range should be positive.");
	my afterRange.first = std::min (from, to);
	my afterRange.last = std::max (from, to);
}

void TextGridTierNavigator_modifyTopicCriterion (TextGridTierNavigator me, kMelder_string newCriterion, kMatchBoolean matchBoolean) {
	NavigationContext_modifyTopicCriterion (my navigationContext.get(), newCriterion, matchBoolean);
}

void TextGridTierNavigator_modifyBeforeCriterion (TextGridTierNavigator me, kMelder_string newCriterion, kMatchBoolean matchBoolean) {
	NavigationContext_modifyBeforeCriterion (my navigationContext.get(), newCriterion, matchBoolean);
}

void TextGridTierNavigator_modifyAfterCriterion (TextGridTierNavigator me, kMelder_string newCriterion, kMatchBoolean matchBoolean) {
	NavigationContext_modifyAfterCriterion (my navigationContext.get(), newCriterion, matchBoolean);
}

void TextGridTierNavigator_modifyUseCriterion (TextGridTierNavigator me, kContext_use newUse, bool excludeTopicMatch) {
	NavigationContext_modifyUseCriterion (my navigationContext.get(), newUse, excludeTopicMatch);
}

static bool TextGridTierNavigator_isTopicMatch (TextGridTierNavigator me, integer index) {
	conststring32 label = my v_getLabel (index);
	return NavigationContext_isTopicLabel (my navigationContext.get(), label);
}

integer TextGridTierNavigator_findBeforeIndex (TextGridTierNavigator me, integer topicIndex) {
	if (! my navigationContext -> beforeLabels)
		return 0;
	if (topicIndex - my beforeRange.first < 1 || topicIndex > my v_getSize ())
		return 0;
	const integer startIndex = std::max (1_integer, topicIndex - my beforeRange.first);
	const integer endIndex = std::max (1_integer, topicIndex - my beforeRange.last);
	for (integer index = startIndex; index >= endIndex; index --) {
		conststring32 label = my v_getLabel (index);
		if (NavigationContext_isBeforeLabel (my navigationContext.get(), label))
			return index;
	}
	return 0;
}

integer TextGridTierNavigator_findAfterIndex (TextGridTierNavigator me, integer topicIndex) {
	if (! my navigationContext -> afterLabels)
		return 0;
	const integer mySize = my v_getSize ();
	if (topicIndex + my afterRange.first > mySize || topicIndex < 1)
		return 0;
	const integer startInterval = std::min (mySize, topicIndex + my afterRange.last);
	const integer endInterval = std::min (mySize, topicIndex + my afterRange.last);
	for (integer index = startInterval; index <= endInterval; index ++) {
		conststring32 label = my v_getLabel (index);
		if (NavigationContext_isAfterLabel (my navigationContext.get(), label))
			return index;
	}
	return 0;
}

integer TextGridTierNavigator_getNumberOfAfterMatches (TextGridTierNavigator me) {
	if (my navigationContext -> afterLabels -> numberOfStrings == 0)
		return 0;
	integer numberOfMatches = 0;
	for (integer index = 1; index <= my v_getSize (); index ++) {
		conststring32 label = my v_getLabel (index);
		if (NavigationContext_isAfterLabel (my navigationContext.get(), label))
			numberOfMatches ++;
	}
	return numberOfMatches;
}

integer TextGridTierNavigator_getNumberOfBeforeMatches (TextGridTierNavigator me) {
	if (my navigationContext -> beforeLabels -> numberOfStrings == 0)
		return 0;
	integer numberOfMatches = 0;
	for (integer index = 1; index <= my v_getSize (); index ++) {
		conststring32 label = my v_getLabel (index);
		if (NavigationContext_isBeforeLabel (my navigationContext.get(), label))
			numberOfMatches ++;
	}
	return numberOfMatches;
}

integer TextGridTierNavigator_getNumberOfTopicOnlyMatches (TextGridTierNavigator me) {
	if (my navigationContext -> topicLabels -> numberOfStrings == 0)
		return 0;
	integer numberOfMatches = 0;
	for (integer index = 1; index <= my v_getSize (); index ++) {
		conststring32 label = my v_getLabel (index);
		if (NavigationContext_isTopicLabel (my navigationContext.get(), label))
			numberOfMatches ++;
	}
	return numberOfMatches;
}

bool TextGridTierNavigator_isMatch (TextGridTierNavigator me, integer topicIndex) {
	if (topicIndex < 1 && topicIndex > my v_getSize ())
		return false;
	const bool isTopicMatch = ( my navigationContext -> excludeTopicMatch ? true : TextGridTierNavigator_isTopicMatch (me, topicIndex) );
	if (! isTopicMatch || my navigationContext -> useCriterion == kContext_use::NO_BEFORE_AND_NO_AFTER)
		return isTopicMatch;
	bool isMatch = false;
	if (my navigationContext -> useCriterion == kContext_use::BEFORE_AND_AFTER)
		isMatch = TextGridTierNavigator_isBeforeMatch (me, topicIndex) &&
			TextGridTierNavigator_isAfterMatch (me, topicIndex);
	else if (my navigationContext -> useCriterion == kContext_use::AFTER)
		isMatch = TextGridTierNavigator_isAfterMatch (me, topicIndex);
	else if (my navigationContext -> useCriterion == kContext_use::BEFORE)
		isMatch = TextGridTierNavigator_isBeforeMatch (me, topicIndex);
	else if (my navigationContext -> useCriterion == kContext_use::BEFORE_OR_AFTER_OR_BOTH)
		isMatch = TextGridTierNavigator_isBeforeMatch (me, topicIndex) ||
			TextGridTierNavigator_isAfterMatch (me, topicIndex);
	else if (my navigationContext -> useCriterion == kContext_use::BEFORE_OR_AFTER_NOT_BOTH)
		isMatch = TextGridTierNavigator_isBeforeMatch (me, topicIndex) != TextGridTierNavigator_isAfterMatch (me, topicIndex);
	return isMatch;
}

integer TextGridTierNavigator_getNumberOfMatches (TextGridTierNavigator me) {
	integer numberOfMatches = 0;
	for (integer index = 1; index <= my v_getSize (); index ++)
		if (TextGridTierNavigator_isMatch (me, index))
			numberOfMatches ++;
	return numberOfMatches;
}

integer TextGridTierNavigator_getNumberOfTopicMatches (TextGridTierNavigator me) {
	integer numberOfMatches = 0;
	for (integer index = 1; index <= my v_getSize (); index ++)
		if (TextGridTierNavigator_isTopicMatch (me, index))
			numberOfMatches ++;
	return numberOfMatches;
}

static void TextGridTierNavigator_getMatchDomain (TextGridTierNavigator me, integer index, double *out_startTime, double *out_endTime) {
	kContext_where startWhere, endWhere;
	if (my matchDomain == kMatchDomain::MATCH_START_TO_MATCH_END) {
		if (my navigationContext -> useCriterion == kContext_use::NO_BEFORE_AND_NO_AFTER) {
			startWhere = kContext_where::TOPIC;
			endWhere = kContext_where::TOPIC;
		} else if (my navigationContext -> useCriterion == kContext_use::BEFORE) {
			startWhere = kContext_where::BEFORE;
			endWhere = ( my navigationContext -> excludeTopicMatch ? kContext_where::BEFORE : kContext_where::TOPIC );
		} else if (my navigationContext -> useCriterion == kContext_use::AFTER) {
			startWhere = ( my navigationContext -> excludeTopicMatch ? kContext_where::AFTER : kContext_where::TOPIC );
			endWhere = kContext_where::AFTER;
		} else if (my navigationContext -> useCriterion == kContext_use::BEFORE_AND_AFTER) {
			startWhere = kContext_where::BEFORE;
			endWhere = kContext_where::AFTER;
		} else if (my navigationContext -> useCriterion == kContext_use::BEFORE_OR_AFTER_NOT_BOTH) {
			if (TextGridTierNavigator_isBeforeMatch (me, index)) {
				startWhere = kContext_where::BEFORE;
				endWhere = ( my navigationContext -> excludeTopicMatch ? kContext_where::BEFORE : kContext_where::TOPIC );
			} else {
				startWhere = ( my navigationContext -> excludeTopicMatch ? kContext_where::AFTER : kContext_where::TOPIC );
				endWhere = kContext_where::AFTER;
			}
		} else if (my navigationContext -> useCriterion == kContext_use::BEFORE_OR_AFTER_OR_BOTH) {
			bool isBeforeMatch = TextGridTierNavigator_isBeforeMatch (me, index);
			bool isAfterMatch = TextGridTierNavigator_isAfterMatch (me, index);
			if (isBeforeMatch && isAfterMatch) {
				startWhere = kContext_where::BEFORE;
				endWhere = kContext_where::AFTER;
			} else if (isBeforeMatch) {
				startWhere = kContext_where::BEFORE;
				endWhere = ( my navigationContext -> excludeTopicMatch ? kContext_where::BEFORE : kContext_where::TOPIC );
			} else {
				startWhere = ( my navigationContext -> excludeTopicMatch ? kContext_where::AFTER : kContext_where::TOPIC );
				endWhere = kContext_where::AFTER;
			}
		}
	} else if (my matchDomain == kMatchDomain::TOPIC_START_TO_TOPIC_END) {
		startWhere = kContext_where::TOPIC;
		endWhere = kContext_where::TOPIC;
	} else if (my matchDomain == kMatchDomain::BEFORE_START_TO_TOPIC_END) {
		startWhere = kContext_where::BEFORE;
		endWhere = kContext_where::TOPIC;
	} else if (my matchDomain == kMatchDomain::TOPIC_START_TO_AFTER_END) {
		startWhere = kContext_where::TOPIC;
		endWhere = kContext_where::AFTER;
	} else if (my matchDomain == kMatchDomain::BEFORE_START_TO_AFTER_END) {
		startWhere = kContext_where::BEFORE;
		endWhere = kContext_where::AFTER;
	} else if (my matchDomain == kMatchDomain::BEFORE_START_TO_BEFORE_END) {
		startWhere = kContext_where::BEFORE;
		endWhere = kContext_where::BEFORE;
	} else if (my matchDomain == kMatchDomain::AFTER_START_TO_AFTER_END) {
		startWhere = kContext_where::AFTER;
		endWhere = kContext_where::AFTER;
	}
	if (out_startTime)
		*out_startTime = TextGridTierNavigator_getStartTime (me, startWhere);
	if (out_endTime)
		*out_endTime = TextGridTierNavigator_getEndTime (me, endWhere);
}

static integer TextGridTierNavigator_setCurrentAtTime (TextGridTierNavigator me, double time) {
	my currentTopicIndex = my v_timeToIndex (time);
	return my currentTopicIndex;
}

integer TextGridTierNavigator_findNext (TextGridTierNavigator me) {
	const integer currentTopicIndex = my currentTopicIndex, size = my v_getSize ();
	for (integer index = currentTopicIndex + 1; index <= size; index ++) {
		if (TextGridTierNavigator_isMatch (me, index)) {
			my currentTopicIndex = index;
			return index;
		}
	}
	my currentTopicIndex = size + 1; // offRight
	return my currentTopicIndex;
}

integer TextGridTierNavigator_findNextAfterTime (TextGridTierNavigator me, double time) {
	TextGridTierNavigator_setCurrentAtTime (me, time);
	return TextGridTierNavigator_findNext (me);
}

integer TextGridTierNavigator_findPrevious (TextGridTierNavigator me) {
	const integer currentTopicIndex = my currentTopicIndex;
	for (integer index = currentTopicIndex - 1; index > 0; index --) {
		if (TextGridTierNavigator_isMatch (me, index)) {
			my currentTopicIndex = index;
			return index;
		}
	}
	my currentTopicIndex = 0;
	return 0;
}

integer TextGridTierNavigator_findPreviousBeforeTime (TextGridTierNavigator me, double time) {
	TextGridTierNavigator_setCurrentAtTime (me, time);
	return TextGridTierNavigator_findPrevious (me);
}

integer TextGridTierNavigator_getIndex (TextGridTierNavigator me, kContext_where where) {
	if (my currentTopicIndex == 0 || my currentTopicIndex > my v_getSize ())
		return 0;
	const integer index = ( where == kContext_where::TOPIC ? my currentTopicIndex :
		where == kContext_where::BEFORE ? TextGridTierNavigator_findBeforeIndex (me, my currentTopicIndex) : 
		where == kContext_where::AFTER ? TextGridTierNavigator_findAfterIndex (me, my currentTopicIndex) : 0);
	return ( index > my v_getSize () ? 0 : index );
}

double TextGridTierNavigator_getStartTime (TextGridTierNavigator me, kContext_where where) {
	const integer index = TextGridTierNavigator_getIndex (me, where);
	return my v_getStartTime (index);
}

conststring32 TextGridTierNavigator_getLabel (TextGridTierNavigator me, kContext_where where) {
	const integer index = TextGridTierNavigator_getIndex (me, where);
	return my v_getLabel (index);
}

double TextGridTierNavigator_getEndTime (TextGridTierNavigator me, kContext_where where) {
	const integer index = TextGridTierNavigator_getIndex (me, where);
	return my v_getEndTime (index);
}

/* End of file TextGridTierNavigator.cpp */