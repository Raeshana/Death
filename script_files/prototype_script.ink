// TAG LIST:
// pause: stop until prompted to. 


VAR start_abigail = false
VAR start_sherry = false
VAR start_keith = false

VAR complete_abigail = false
VAR complete_sherry = false
VAR complete_keith = false

VAR battle_difficulty = 3



Welcome to your first day on the job.
To make things easy for you, we have stationed you in a hospital.
In order to make today's quota, you must reap at least 1 (one) soul.
As you may or may not know, 
reapers have the flexibility to choose their own method of collection. 
When you attempt to collect their souls,
Humans on the verge of death are acutely aware of your presence.
Therefore, you may chose to drag your target to the depths
or convince your target to be collected. 
What you choose is up to you. # pause

-> henry


=== henry ===
    ~ temp checked_diary = false
    ~ battle_difficulty = 3
    This is your first client.
    If you're in a rush, you may directly apporoach to reap his soul
    Or, you may choose to know more about him to make it easier. 
    { checked_diary:
        DIARY CONTENTS: 
        "Stacy and the grandbabies visited me again. 
        Jack turned 14 yesterday and he starts high school soon... 
        Alice is entering the third grade and she adores her friends. 
        They are both growing so big. Andrew couldn't visit because of his work. 
        He should spend more time with them.
        Stacy looks just like Dee when she was younger. 
        If only Dee could see them now." 
        ~ battle_difficulty = 1
    }
    * "It is time."
    { checked_diary:
            "'It is time'...? Ah, are you death? I can't go...
            I am not ready. Everyone still needs me...
            But you read my diary so I dont mind"
    - else: 
        "'It is time'...? Ah, are you death? I can't go...
        I am not ready. Everyone still needs me...
        But you didnt read my diary so I'll put up a fight!"
    } # battle
    
    
Congraulations on your first capture.
Hopefully it wasn't that difficuly for you.
I think that you'll do just fine out there.
Good luck.

-> home
=== home ===
-> sherry
# pause
{ 
    - start_abigail && not complete_abigail:
        -> abigail
    - start_keith && not complete_keith: 
        -> keith
    - start_sherry && not complete_sherry:
        -> sherry
}


-> abigail

=== abigail ===
    ~ temp checked_diary = false
    ~ battle_difficulty = 3
    
    { checked_diary: 
        This hurts!
    }
    * "It is time." 
    - "Time...? But I'm can't!"
    - "I havent finished my dissertation."
    - "I... I must finish."
    # battle
    ~ complete_abigail = true
    -> home
    
=== sherry ===
    ~ temp checked_diary = false
    ~ battle_difficulty = 4
    { checked_diary: 
        I have fought a good fight, 
        I have finished my course, 
        I have kept the faith:
        henceforth there is laid up for me a crown of righteousness, 
        which the Lord, the righteous judge, 
        shall give me at that day: 
        and not to me only, 
        but unto all them also that love his appearing.
        \-- 2 Timothy 4:7-8.
        Every day I grow more weary. 
        It may be time for this ol' Sherry to join the rest of my friends.
    }
    * "It is time." 
    - "Ahhh my Lord, my God. The Father, Son, and The Holy Spirit."
    - "Have you come to lead me to the afterlife?"
    * "One can say that."
    - "You... are not my savior."
    - "The way you are dressed..."
    - "... and that ferioucios weapon..!"
    - "You... must be Satan. God cannot look like this"
    { checked_diary: 
        * You cannot see my face, for no one may see me and live.
            -> success
        * The Lord may take many forms, Sherry. This is the one I choose for you.
            -> weak
        * I am both, yet neither, God nor Satan.
            -> fail
    }
    = success
        "Exodus 33:20... What the Lord said to Moses before he led the people to Isreal"
        "I understand that is is you my Lord,"
        "and that I was wrong to doubt you."
        ~ battle_difficulty = 1
        # battle 
        -> fight

    = weak
        "The form for me? That is peculiar."
        "But I shouldn't doubt God's plan..."
        ~ battle_difficulty = 3
        # battle
        -> fight

        
    = fail
        "That is absolutely sacriligious."
        "Only a demon of Satan would conflate God and the Devil."
        ~ battle_difficulty = 5
        # battle
        -> fight
    
    = fight
        # battle
        ~ complete_sherry = true
        -> home

=== keith ===
    ~ temp checked_diary = false
    ~ battle_difficulty = 1
    { checked_diary:
        DIARY CONTENTS: 
        "finally an escape from my suffering
        an excuse from everything in my life
        if im here then everyone will be fine if i do nothing
        i dont need to worry
        i just wnat to sleep foreever
        i can finally rest
        im done 
        im done im doneimdoneimdone 
        imdoneimdoneimdoneiwanttodisappear
        its time to disappear"
        ~ battle_difficulty = 1
    }
    "...yea, sure, I'm fine with this."
    * "...W-what?"
    - "Youre Death right?"
    * "One can say that."
    - "Okay, perfect, sure, I'm good to go. So how does this thing work?"
    * "Well I peer into your soul, the thread that connects your conciousne--"
    - "Bro actually, I kinda dont gaf, can we just get this over with?"
    { checked_diary: 
        -> diary
        - else: 
        # battle
    }
    = diary
    * "Whatever you wish."
        -> fight
    * "I can't reap you." 
        -> reject
    = fight 
        # battle
        -> home

    = reject
        - "..."
        - "Am I really that pathetic that Death himself is rejecting me?"
        * "Well - no - you're just unwell, and need help and I cannot give you that."
        - "... how persumptuous of you to think you know what I need."
        - "Just leave me be."
    ~complete_keith = true
    -> home
    
    
    

  

    




-> END
